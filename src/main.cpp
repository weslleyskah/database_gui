#include "vulkan.h"
#include "human.h"
#include "font_roboto.h"
#include "sqlite3.h"

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <stdexcept>
#include <filesystem>

using namespace std;

// ===========================================================================================================
// Database
// ===========================================================================================================

void insert_registry_sqlite(sqlite3* db, pessoa* p) {
    const char* sql = "INSERT INTO registries (type, nome, address, telefone, credito, divida, "
                      "codigoSetor, salarioBase, imposto, ajudaDeCusto, valorProducao, comissao, valorVendas) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Prepare insert failed: " << sqlite3_errmsg(db) << endl;
        return;
    }

    if (auto f = dynamic_cast<fornecedor*>(p)) {
        sqlite3_bind_text(stmt, 1, "Fornecedor", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, f->getNome().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, f->getAddress().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, f->getTelefone().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 5, f->getCredito());
        sqlite3_bind_int(stmt, 6, f->getDivida());
    } else if (auto a = dynamic_cast<admin*>(p)) {
        sqlite3_bind_text(stmt, 1, "Admin", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, a->getNome().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, a->getAddress().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, a->getTelefone().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_null(stmt, 5);
        sqlite3_bind_null(stmt, 6);
        sqlite3_bind_int(stmt, 7, a->getCodigoSetor());
        sqlite3_bind_int(stmt, 8, a->getSalarioBase());
        sqlite3_bind_double(stmt, 9, a->getImposto());
        sqlite3_bind_int(stmt, 10, a->getAjudaDeCusto());
    } else if (auto o = dynamic_cast<operario*>(p)) {
        sqlite3_bind_text(stmt, 1, "Operario", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, o->getNome().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, o->getAddress().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, o->getTelefone().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_null(stmt, 5);
        sqlite3_bind_null(stmt, 6);
        sqlite3_bind_int(stmt, 7, o->getCodigoSetor());
        sqlite3_bind_int(stmt, 8, o->getSalarioBase());
        sqlite3_bind_double(stmt, 9, o->getImposto());
        sqlite3_bind_null(stmt, 10);
        sqlite3_bind_int(stmt, 11, o->getValorProducao());
        sqlite3_bind_int(stmt, 12, o->getComissao());
    } else if (auto v = dynamic_cast<vendedor*>(p)) {
        sqlite3_bind_text(stmt, 1, "Vendedor", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, v->getNome().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, v->getAddress().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, v->getTelefone().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_null(stmt, 5);
        sqlite3_bind_null(stmt, 6);
        sqlite3_bind_int(stmt, 7, v->getCodigoSetor());
        sqlite3_bind_int(stmt, 8, v->getSalarioBase());
        sqlite3_bind_double(stmt, 9, v->getImposto());
        sqlite3_bind_null(stmt, 10);
        sqlite3_bind_null(stmt, 11);
        sqlite3_bind_int(stmt, 12, v->getComissao());
        sqlite3_bind_int(stmt, 13, v->getValorVendas());
    } else if (auto e = dynamic_cast<empregado*>(p)) {
        sqlite3_bind_text(stmt, 1, "Empregado", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, e->getNome().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, e->getAddress().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, e->getTelefone().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_null(stmt, 5);
        sqlite3_bind_null(stmt, 6);
        sqlite3_bind_int(stmt, 7, e->getCodigoSetor());
        sqlite3_bind_int(stmt, 8, e->getSalarioBase());
        sqlite3_bind_double(stmt, 9, e->getImposto());
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Step insert failed: " << sqlite3_errmsg(db) << endl;
    }
    sqlite3_finalize(stmt);
}

void delete_registry_sqlite(sqlite3* db, const string& nome, const string& address, const string& telefone) {
    const char* sql = "DELETE FROM registries WHERE nome = ? AND address = ? AND telefone = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Prepare delete failed: " << sqlite3_errmsg(db) << endl;
        return;
    }
    
    sqlite3_bind_text(stmt, 1, nome.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, address.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, telefone.c_str(), -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Step delete failed: " << sqlite3_errmsg(db) << endl;
    }
    sqlite3_finalize(stmt);
}

void load_database_sqlite(sqlite3* db, vector<pessoa*>& pessoas) {
    for (auto p : pessoas) {
        delete p;
    }
    pessoas.clear();
    
    const char* sql = "SELECT type, nome, address, telefone, credito, divida, "
                      "codigoSetor, salarioBase, imposto, ajudaDeCusto, valorProducao, comissao, valorVendas "
                      "FROM registries;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Prepare select failed: " << sqlite3_errmsg(db) << endl;
        return;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* nome = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* telefone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        
        string s_type = type ? type : "";
        string s_nome = nome ? nome : "";
        string s_address = address ? address : "";
        string s_telefone = telefone ? telefone : "";
        
        if (s_type == "Fornecedor") {
            int credito = sqlite3_column_int(stmt, 4);
            int divida = sqlite3_column_int(stmt, 5);
            pessoas.push_back(new fornecedor(s_nome, s_address, s_telefone, credito, divida));
        } else if (s_type == "Empregado") {
            int sector = sqlite3_column_int(stmt, 6);
            int sal = sqlite3_column_int(stmt, 7);
            float imp = static_cast<float>(sqlite3_column_double(stmt, 8));
            pessoas.push_back(new empregado(s_nome, s_address, s_telefone, sector, sal, imp));
        } else if (s_type == "Admin") {
            int sector = sqlite3_column_int(stmt, 6);
            int sal = sqlite3_column_int(stmt, 7);
            float imp = static_cast<float>(sqlite3_column_double(stmt, 8));
            int allowance = sqlite3_column_int(stmt, 9);
            pessoas.push_back(new admin(s_nome, s_address, s_telefone, sector, sal, imp, allowance));
        } else if (s_type == "Operario") {
            int sector = sqlite3_column_int(stmt, 6);
            int sal = sqlite3_column_int(stmt, 7);
            float imp = static_cast<float>(sqlite3_column_double(stmt, 8));
            int prod = sqlite3_column_int(stmt, 10);
            int comm = sqlite3_column_int(stmt, 11);
            pessoas.push_back(new operario(s_nome, s_address, s_telefone, sector, sal, imp, prod, comm));
        } else if (s_type == "Vendedor") {
            int sector = sqlite3_column_int(stmt, 6);
            int sal = sqlite3_column_int(stmt, 7);
            float imp = static_cast<float>(sqlite3_column_double(stmt, 8));
            int sales = sqlite3_column_int(stmt, 12);
            int comm = sqlite3_column_int(stmt, 11);
            pessoas.push_back(new vendedor(s_nome, s_address, s_telefone, sector, sal, imp, sales, comm));
        }
    }
    sqlite3_finalize(stmt);
}

bool init_database(const string& db_file, sqlite3*& db, vector<pessoa*>& pessoas) {
    filesystem::path p(db_file);
    if (p.has_parent_path()) {
        filesystem::create_directories(p.parent_path());
    }

    if (sqlite3_open(db_file.c_str(), &db) != SQLITE_OK) {
        cerr << "Failed to open database: " << sqlite3_errmsg(db) << endl;
        return false;
    }
    
    const char* sql_create = "CREATE TABLE IF NOT EXISTS registries ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                             "type TEXT NOT NULL, "
                             "nome TEXT NOT NULL, "
                             "address TEXT NOT NULL, "
                             "telefone TEXT NOT NULL, "
                             "credito INTEGER, "
                             "divida INTEGER, "
                             "codigoSetor INTEGER, "
                             "salarioBase INTEGER, "
                             "imposto REAL, "
                             "ajudaDeCusto INTEGER, "
                             "valorProducao INTEGER, "
                             "comissao INTEGER, "
                             "valorVendas INTEGER"
                             ");";
    
    char* err_msg = nullptr;
    if (sqlite3_exec(db, sql_create, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        cerr << "Failed to create table: " << err_msg << endl;
        sqlite3_free(err_msg);
        return false;
    }
    
    const char* sql_count = "SELECT COUNT(*) FROM registries;";
    sqlite3_stmt* stmt;
    int count = 0;
    if (sqlite3_prepare_v2(db, sql_count, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    
    if (count == 0) {
        vector<pessoa*> mock_entries;
        mock_entries.push_back(new fornecedor("Matrix", "01 Neo Way", "000", 10000, 2000));
        mock_entries.push_back(new empregado("Neo", "02 Neo Way", "000", 10, 5000, 10.0f));
        mock_entries.push_back(new admin("Morpheus", "03 Neo Way", "000", 10, 8000, 15.0f, 1500));
        mock_entries.push_back(new vendedor("Trinity", "04 Neo Way", "000", 10, 6000, 12.0f, 4000, 10));
        
        for (auto item : mock_entries) {
            insert_registry_sqlite(db, item);
            delete item;
        }
    }
    
    load_database_sqlite(db, pessoas);
    return true;
}

// ===========================================================================================================
// GUI Style
// ===========================================================================================================

void PushWindowStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.09f, 0.12f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.18f, 0.22f, 0.33f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.24f, 0.29f, 0.44f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.15f, 0.18f, 0.28f, 1.00f));
}

void PushButtonStyle() {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.38f, 0.73f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.48f, 0.88f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.32f, 0.65f, 1.00f));
}

void PopButtonStyle() {
    ImGui::PopStyleColor(3);
}

void PopWindowStyle() {
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(4);
}

// ===========================================================================================================
// GUI
// ===========================================================================================================

int main() {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "GUI", nullptr, nullptr);

    uint32_t extensions_count = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
    SetupVulkan(extensions, extensions_count);

    VkSurfaceKHR surface;
    check_vk_result(glfwCreateWindowSurface(g_Instance, window, g_Allocator, &surface));

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
    SetupVulkanWindow(wd, surface, w, h);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    // Load font

    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;
    
    ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF(
        const_cast<uint8_t*>(g_RobotoRegular), sizeof(g_RobotoRegular), 18.0f, &font_cfg
    );
    
    if (!robotoFont) {
        std::cerr << "Warning: Could not load embedded Roboto font. Using default.\n";
    } else {
        io.FontDefault = robotoFont;
    }

    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance        = g_Instance;
    init_info.PhysicalDevice  = g_PhysicalDevice;
    init_info.Device          = g_Device;
    init_info.QueueFamily     = g_QueueFamily;
    init_info.Queue           = g_Queue;
    init_info.DescriptorPool  = g_DescriptorPool;
    init_info.MinImageCount   = (uint32_t)g_MinImageCount;
    init_info.ImageCount      = wd->ImageCount;
    init_info.Allocator       = g_Allocator;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

    // Upload fonts
    {
        VkCommandBuffer command_buffer = wd->Frames[wd->FrameIndex].CommandBuffer;
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(command_buffer, &begin_info);

        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

        VkSubmitInfo end_info = {};
        end_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers    = &command_buffer;
        vkEndCommandBuffer(command_buffer);
        vkQueueSubmit(g_Queue, 1, &end_info, VK_NULL_HANDLE);

        vkDeviceWaitIdle(g_Device);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    std::vector<pessoa*> pessoas;
    sqlite3* db = nullptr;
    const std::string db_file = "database/database.db";
    init_database(db_file, db, pessoas);

    // --- Main loop ---
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        if (width == 0 || height == 0)
            continue;

        if (g_SwapChainRebuild) {
            ImGui_ImplVulkan_SetMinImageCount((uint32_t)g_MinImageCount);
            ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device,
                &g_MainWindowData, g_QueueFamily, g_Allocator, width, height, (uint32_t)g_MinImageCount);
            g_MainWindowData.FrameIndex = 0;
            g_SwapChainRebuild = false;
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::GetIO().IniFilename = nullptr;

        PushWindowStyle();
        PushButtonStyle();
        // Begin the main dashboard window
        ImGui::Begin("Person Registry System", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        
        ImGui::Text("System Registry Dashboard");
        ImGui::Separator();
        ImGui::Spacing();
        
        // Input state buffers persisting across frames
        static char name_buf[128] = "";
        static char addr_buf[128] = "";
        static char phone_buf[64] = "";
        static int input_credito = 0;
        static int input_divida = 0;
        static int input_sector = 0;
        static int input_sal_base = 0;
        static float input_imposto = 0.0f;
        static int input_allowance = 0;
        static int input_prod_val = 0;
        static int input_commission = 0;
        static int input_sales_val = 0;
        static int input_vendedor_commission = 0;
        
        const char* types[] = { "Fornecedor", "Empregado", "Admin", "Operario", "Vendedor" };
        static int selected_type_idx = 0;
        
        if (ImGui::BeginTabBar("MainTabs")) {
            if (ImGui::BeginTabItem("Registry Overview")) {
                if (ImGui::BeginTable("PeopleTable", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
                    ImGui::TableSetupColumn("Name");
                    ImGui::TableSetupColumn("Role/Type");
                    ImGui::TableSetupColumn("Address");
                    ImGui::TableSetupColumn("Phone");
                    ImGui::TableSetupColumn("Base Salary / Credit");
                    ImGui::TableSetupColumn("Specifics");
                    ImGui::TableSetupColumn("Net Salary / Balance");
                    ImGui::TableSetupColumn("Action");
                    ImGui::TableHeadersRow();
                    
                    int to_delete = -1;
                    for (size_t i = 0; i < pessoas.size(); ++i) {
                        pessoa* p = pessoas[i];
                        ImGui::TableNextRow();
                        
                        // Column 0: Name
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%s", p->getNome().c_str());
                        
                        // Column 1: Role/Type
                        ImGui::TableSetColumnIndex(1);
                        const char* role_str = "Unknown";
                        if (dynamic_cast<fornecedor*>(p)) role_str = "Fornecedor";
                        else if (dynamic_cast<admin*>(p)) role_str = "Admin";
                        else if (dynamic_cast<operario*>(p)) role_str = "Operario";
                        else if (dynamic_cast<vendedor*>(p)) role_str = "Vendedor";
                        else if (dynamic_cast<empregado*>(p)) role_str = "Empregado";
                        ImGui::Text("%s", role_str);
                        
                        // Column 2: Address
                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("%s", p->getAddress().c_str());
                        
                        // Column 3: Phone
                        ImGui::TableSetColumnIndex(3);
                        ImGui::Text("%s", p->getTelefone().c_str());
                        
                        // Column 4: Base / Credit
                        ImGui::TableSetColumnIndex(4);
                        if (auto f = dynamic_cast<fornecedor*>(p)) {
                            ImGui::Text("Credit: %d", f->getCredito());
                        } else if (auto e = dynamic_cast<empregado*>(p)) {
                            ImGui::Text("Base Salary: %d", e->getSalarioBase());
                        }
                        
                        // Column 5: Specific Attributes
                        ImGui::TableSetColumnIndex(5);
                        if (auto f = dynamic_cast<fornecedor*>(p)) {
                            ImGui::Text("Debt: %d", f->getDivida());
                        } else if (auto a = dynamic_cast<admin*>(p)) {
                            ImGui::Text("Sector Code: %d, Tax: %.1f%%, Allowance: %d", a->getCodigoSetor(), a->getImposto(), a->getAjudaDeCusto());
                        } else if (auto o = dynamic_cast<operario*>(p)) {
                            ImGui::Text("Sector Code: %d, Tax: %.1f%%, Production Value: %d, Commission: %d%%", o->getCodigoSetor(), o->getImposto(), o->getValorProducao(), o->getComissao());
                        } else if (auto v = dynamic_cast<vendedor*>(p)) {
                            ImGui::Text("Sector Code: %d, Tax: %.1f%%, Sales Value: %d, Commission: %d%%", v->getCodigoSetor(), v->getImposto(), v->getValorVendas(), v->getComissao());
                        } else if (auto e = dynamic_cast<empregado*>(p)) {
                            ImGui::Text("Sector Code: %d, Tax: %.1f%%", e->getCodigoSetor(), e->getImposto());
                        }
                        
                        // Column 6: Calculated Net Output
                        ImGui::TableSetColumnIndex(6);
                        if (auto f = dynamic_cast<fornecedor*>(p)) {
                            ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.35f, 1.0f), "Balance: %d", f->getSaldo());
                        } else if (auto e = dynamic_cast<empregado*>(p)) {
                            ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.35f, 1.0f), "Net Salary: %.2f", e->calcularSalario());
                        }
                        
                        // Column 7: Delete Button
                        ImGui::TableSetColumnIndex(7);
                        ImGui::PushID(static_cast<int>(i));
                        if (ImGui::Button("Delete")) {
                            to_delete = static_cast<int>(i);
                        }
                        ImGui::PopID();
                    }
                    ImGui::EndTable();
                    
                    if (to_delete != -1) {
                        delete_registry_sqlite(db, pessoas[to_delete]->getNome(), pessoas[to_delete]->getAddress(), pessoas[to_delete]->getTelefone());
                        delete pessoas[to_delete];
                        pessoas.erase(pessoas.begin() + to_delete);
                    }
                } else {
                    ImGui::TextColored(ImVec4(0.8f, 0.3f, 0.3f, 1.0f), "Failed to render registry table.");
                }
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Add New Registry")) {
                ImGui::Combo("Select Type", &selected_type_idx, types, IM_ARRAYSIZE(types));
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::InputText("Name", name_buf, IM_ARRAYSIZE(name_buf));
                ImGui::InputText("Address", addr_buf, IM_ARRAYSIZE(addr_buf));
                ImGui::InputText("Phone", phone_buf, IM_ARRAYSIZE(phone_buf));
                
                if (selected_type_idx == 0) { // Fornecedor
                    ImGui::InputInt("Credit", &input_credito);
                    ImGui::InputInt("Debt", &input_divida);
                } else { // Empregado and subclasses
                    ImGui::InputInt("Sector Code", &input_sector);
                    ImGui::InputInt("Base Salary", &input_sal_base);
                    ImGui::InputFloat("Tax (%)", &input_imposto);
                    
                    if (selected_type_idx == 2) { // Admin
                        ImGui::InputInt("Allowance (Ajuda de custo)", &input_allowance);
                    } else if (selected_type_idx == 3) { // Operario
                        ImGui::InputInt("Production Value", &input_prod_val);
                        ImGui::InputInt("Commission (%)", &input_commission);
                    } else if (selected_type_idx == 4) { // Vendedor
                        ImGui::InputInt("Sales Value", &input_sales_val);
                        ImGui::InputInt("Commission (%)", &input_vendedor_commission);
                    }
                }
                
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                
                if (ImGui::Button("Add Registry Entry", ImVec2(180, 40))) {
                    pessoa* new_p = nullptr;
                    if (selected_type_idx == 0) {
                        new_p = new fornecedor(name_buf, addr_buf, phone_buf, input_credito, input_divida);
                    } else if (selected_type_idx == 1) {
                        new_p = new empregado(name_buf, addr_buf, phone_buf, input_sector, input_sal_base, input_imposto);
                    } else if (selected_type_idx == 2) {
                        new_p = new admin(name_buf, addr_buf, phone_buf, input_sector, input_sal_base, input_imposto, input_allowance);
                    } else if (selected_type_idx == 3) {
                        new_p = new operario(name_buf, addr_buf, phone_buf, input_sector, input_sal_base, input_imposto, input_prod_val, input_commission);
                    } else if (selected_type_idx == 4) {
                        new_p = new vendedor(name_buf, addr_buf, phone_buf, input_sector, input_sal_base, input_imposto, input_sales_val, input_vendedor_commission);
                    }
                    
                    if (new_p) {
                        insert_registry_sqlite(db, new_p);
                        pessoas.push_back(new_p);
                    }
                    
                    // Reset inputs
                    name_buf[0] = '\0';
                    addr_buf[0] = '\0';
                    phone_buf[0] = '\0';
                    input_credito = 0;
                    input_divida = 0;
                    input_sector = 0;
                    input_sal_base = 0;
                    input_imposto = 0.0f;
                    input_allowance = 0;
                    input_prod_val = 0;
                    input_commission = 0;
                    input_sales_val = 0;
                    input_vendedor_commission = 0;
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

	// ImGui::ShowDemoWindow();

        PopButtonStyle();
        ImGui::End();
        PopWindowStyle();

        ImGui::Render();
        FrameRender(wd, ImGui::GetDrawData());
        FramePresent(wd);
    }

    // --- Cleanup ---
    for (auto p : pessoas) {
        delete p;
    }
    pessoas.clear();
    if (db) {
        sqlite3_close(db);
    }

    vkDeviceWaitIdle(g_Device);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    ImGui_ImplVulkanH_DestroyWindow(g_Instance, g_Device, wd, g_Allocator);
    vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);
    vkDestroyDevice(g_Device, g_Allocator);
    vkDestroyInstance(g_Instance, g_Allocator);

    glfwDestroyWindow(window);
    glfwTerminate();

	return 0;
}