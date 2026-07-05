<?php
// Load database
$possible_paths = [
    __DIR__ . '/../database/database.db',
    __DIR__ . '/../build/Debug/database/database.db',
    __DIR__ . '/../build/Release/database/database.db',
    __DIR__ . '/database.db' 
];

$db_path = null;
foreach ($possible_paths as $path) {
    if (file_exists($path)) {
        $db_path = $path;
        break;
    }
}

$pessoas = [];

if ($db_path) {
    try {
        $pdo = new PDO('sqlite:' . $db_path);
        $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
        
        $stmt = $pdo->query("SELECT * FROM registries");
        $pessoas = $stmt->fetchAll(PDO::FETCH_ASSOC);
    } catch (PDOException $e) {
        $error = "Database Error: " . $e->getMessage();
    }
} else {
    $error = "Database not found. Please run the GUI application first to generate it.";
}

function formatCurrency($value) {
    return is_numeric($value) ? '$' . number_format($value, 2) : '-';
}
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Manager Dashboard - Registries</title>
    <link rel="stylesheet" href="style.css">
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
</head>
<body>
    <div class="dashboard">
        <header class="header">
            <div>
                <h1>System Registry Dashboard</h1>
            </div>
            <div class="stats">
                <div class="stat-card">
                    <span class="stat-value"><?= count($pessoas) ?></span>
                    <span class="stat-label">Total Entries</span>
                </div>
            </div>
        </header>

        <main class="content">
            <?php if (isset($error)): ?>
                <div class="error-alert">
                    <?= htmlspecialchars($error) ?>
                </div>
            <?php else: ?>
                <div class="table-container">
                    <table>
                        <thead>
                            <tr>
                                <th>Name</th>
                                <th>Role/Type</th>
                                <th>Address</th>
                                <th>Phone</th>
                                <th>Base Salary / Credit</th>
                                <th>Specifics</th>
                                <th>Net Salary / Balance</th>
                            </tr>
                        </thead>
                        <tbody>
                            <?php foreach ($pessoas as $p): 
                                $type = $p['type'];
                                $netOutput = 0;
                                $balanceStr = "-";
                                
                                if ($type === 'Fornecedor') {
                                    $netOutput = $p['credito'] - $p['divida'];
                                    $balanceStr = formatCurrency($netOutput);
                                } else {
                                    $base = $p['salarioBase'] ?? 0;
                                    $imposto = $p['imposto'] ?? 0;
                                    $taxDeduction = $base * ($imposto / 100);
                                    
                                    if ($type === 'Admin') {
                                        $netOutput = $base - $taxDeduction + ($p['ajudaDeCusto'] ?? 0);
                                    } elseif ($type === 'Operario') {
                                        $netOutput = $base - $taxDeduction + (($p['valorProducao'] ?? 0) * (($p['comissao'] ?? 0) / 100));
                                    } elseif ($type === 'Vendedor') {
                                        $netOutput = $base - $taxDeduction + (($p['valorVendas'] ?? 0) * (($p['comissao'] ?? 0) / 100));
                                    } elseif ($type === 'Empregado') {
                                        $netOutput = $base - $taxDeduction;
                                    }
                                    $balanceStr = formatCurrency($netOutput);
                                }
                            ?>
                                <tr>
                                    <td class="fw-500"><?= htmlspecialchars($p['nome']) ?></td>
                                    <td><span class="badge badge-<?= strtolower($type) ?>"><?= htmlspecialchars($type) ?></span></td>
                                    <td><?= htmlspecialchars($p['address']) ?></td>
                                    <td><?= htmlspecialchars($p['telefone']) ?></td>
                                    <td>
                                        <?php if ($type === 'Fornecedor'): ?>
                                            <span class="text-muted">Credit:</span> <?= formatCurrency($p['credito']) ?>
                                        <?php else: ?>
                                            <span class="text-muted">Base:</span> <?= formatCurrency($p['salarioBase']) ?>
                                        <?php endif; ?>
                                    </td>
                                    <td>
                                        <div class="specifics">
                                            <?php if ($type === 'Fornecedor'): ?>
                                                <span>Debt: <?= formatCurrency($p['divida']) ?></span>
                                            <?php else: ?>
                                                <span>Sector: <?= htmlspecialchars($p['codigoSetor']) ?></span>
                                                <span>Tax: <?= htmlspecialchars($p['imposto']) ?>%</span>
                                                <?php if ($type === 'Admin'): ?>
                                                    <span>Allowance: <?= formatCurrency($p['ajudaDeCusto']) ?></span>
                                                <?php elseif ($type === 'Operario'): ?>
                                                    <span>Production: <?= formatCurrency($p['valorProducao']) ?></span>
                                                    <span>Commission: <?= htmlspecialchars($p['comissao']) ?>%</span>
                                                <?php elseif ($type === 'Vendedor'): ?>
                                                    <span>Sales: <?= formatCurrency($p['valorVendas']) ?></span>
                                                    <span>Commission: <?= htmlspecialchars($p['comissao']) ?>%</span>
                                                <?php endif; ?>
                                            <?php endif; ?>
                                        </div>
                                    </td>
                                    <td class="text-success fw-600">
                                        <?= $balanceStr ?>
                                    </td>
                                </tr>
                            <?php endforeach; ?>
                            <?php if (empty($pessoas)): ?>
                                <tr>
                                    <td colspan="7" class="text-center py-4">No records found.</td>
                                </tr>
                            <?php endif; ?>
                        </tbody>
                    </table>
                </div>
            <?php endif; ?>
        </main>
    </div>
</body>
</html>
