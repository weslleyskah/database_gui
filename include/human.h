#ifndef humanh
#define humanh

#include <iostream>
#include <string>

using namespace std;

class pessoa {
private:
	string nome;
	string address;
	string telefone;
public:
	pessoa(string n, string e, string t): nome(n), address(e), telefone(t) {}
	/*pessoa(string n, string e, string t) {
		nome = n;
		address = e;
		telefone = t;
	}
	pessoa(string n, string e, string t) {
		this->nome = n;
		this->address = e;
		this->telefone = t;
	}
	*/
	virtual ~pessoa() {}

	string getNome() { return nome; }
	string getAddress() { return address; }
	string getTelefone() { return telefone; }

	void setNome(string n) { nome = n; }
	void setAddress(string e) { address = e; }
	void setTelefone(string t) { telefone = t; }

};

class fornecedor : public pessoa {
private:
	int credito;
	int divida;
public:
	fornecedor(string n, string e, string t, int c, int d) : pessoa(n, e, t), credito(c), divida(d) {}

	int getCredito() { return credito; }
	int getDivida() { return divida; }
	int getSaldo() { return credito - divida; }

	void setCredito(int c) { credito = c; }
	void setDivida(int d) { divida = d; }

};

class empregado : public pessoa {
private:
	int codigoSetor;
	int salarioBase;
	float imposto;
public:
	empregado(string n, string e, string t, int cod, int sal, float imp) : pessoa(n, e, t), codigoSetor(cod), salarioBase(sal), imposto(imp) {}

	int getCodigoSetor() { return codigoSetor; }
	int getSalarioBase() { return salarioBase; }
	float getImposto() { return imposto; }

	void setCodigoSetor(int cod) { codigoSetor = cod; }
	void setSalarioBase(int sal) { salarioBase = sal; }
	void setImposto(float imp) { imposto = imp; }

	virtual float calcularSalario() {
		float salarioLiquido = salarioBase - (imposto / 100 * salarioBase);
		return salarioLiquido;
	}
};

class admin : public empregado {
private:
	int ajudaDeCusto;
public:
	admin(string n, string e, string t, int cod, int sal, float imp, int help) : empregado(n, e, t, cod, sal, imp), ajudaDeCusto(help) {}

	int getAjudaDeCusto() const { return ajudaDeCusto; }
	void setAjudaDeCusto(int help) { ajudaDeCusto = help; }

	float calcularSalario() override {
		float salarioLiquido = empregado::calcularSalario() + ajudaDeCusto;
		return salarioLiquido;
	}
};

class operario : public empregado {
private:
	int valorProducao;
	int comissao;
public:
	operario(string n, string e, string t, int cod, int sal, float imp, int valorProd, int comm) : empregado(n, e, t, cod, sal, imp), valorProducao(valorProd), comissao(comm) {}

	int getValorProducao() { return valorProducao; }
	int getComissao() { return comissao; }

	void setValorProducao(int valorProd) { valorProducao = valorProd; }
	void setComissao(int comm) { comissao = comm; }

	float calcularSalario() override {
		float salarioLiquido = empregado::calcularSalario() + (valorProducao * comissao / 100);
		return salarioLiquido;
	}
};

class vendedor : public empregado {
private:
	int valorVendas;
	int comissao;
public:
	vendedor(string n, string e, string t, int cod, int sal, float imp, int valorVend, int comm) : empregado(n, e, t, cod, sal, imp), valorVendas(valorVend), comissao(comm) {}
	
	int getValorVendas() { return valorVendas; }
	int getComissao() { return comissao; }

	void setValorVendas(int valorVend) { valorVendas = valorVend; }
	void setComissao(int comm) { comissao = comm; }

	float calcularSalario() override {
		float salarioLiquido = empregado::calcularSalario() + (valorVendas * comissao / 100);
		return salarioLiquido;
	}
};

#endif