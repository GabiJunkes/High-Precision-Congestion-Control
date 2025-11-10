import os
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter
from pathlib import Path

# Caminhos base
BASE_DIR = "../simulation/outputs/formated"
OUTPUT_DIR = "graficos"

TIPOS = ["intra", "intra_no_traffic", "inter", "inter_no_traffic"]

# opcional: aplicar escala logarítmica em alguns algoritmos
LOG_SCALE_ALGS = []  # exemplo: ["hpcc", "timely"]

os.makedirs(OUTPUT_DIR, exist_ok=True)

def resumo_estatistico(df: pd.Series):
    """Retorna estatísticas relevantes de uma série numérica"""
    return {
        "media": df.mean(),
        "mediana": df.median(),
        "desvio": df.std(),
        "minimo": df.min(),
        "maximo": df.max(),
    }

def ler_csv_caminho(path):
    dados = []
    if not os.path.exists(path):
        return None
    with open(path, "r") as f:
        for linha in f:
            if linha.startswith("C;"):
                partes = linha.strip().split(";")
                if len(partes) >= 5:
                    cpu = float(partes[1])
                    pacote = int(partes[2])
                    fct = float(partes[3])
                    vazao = float(partes[4])
                    dados.append({"cpu": cpu, "pacote": pacote, "fct": fct, "vazao": vazao})
    return pd.DataFrame(dados)

# Descobre algoritmos
algoritmos = set()
for tipo in TIPOS:
    pasta = os.path.join(BASE_DIR, tipo)
    if os.path.exists(pasta):
        for nome in os.listdir(pasta):
            if nome.startswith("saida_client_") and nome.endswith(".csv"):
                alg = nome.replace("saida_client_", "").replace(".csv", "")
                algoritmos.add(alg)

for alg in sorted(algoritmos):
    print(f"Gerando gráficos e estatísticas para {alg}...")

    dados = {}
    for tipo in TIPOS:
        path = os.path.join(BASE_DIR, tipo, f"saida_client_{alg}.csv")
        df = ler_csv_caminho(path)
        if df is not None and not df.empty:
            dados[tipo] = df

    if not dados:
        continue

    alg_dir = os.path.join(OUTPUT_DIR, alg)
    os.makedirs(alg_dir, exist_ok=True)

    # dicionário para salvar estatísticas agregadas
    resultados_alg = {}

    for base_tipo in ["intra", "inter"]:
        no_traffic = base_tipo + "_no_traffic"
        if base_tipo not in dados or no_traffic not in dados:
            continue

        tipo_dir = os.path.join(alg_dir, base_tipo)
        os.makedirs(tipo_dir, exist_ok=True)

        pacotes = sorted(set(dados[base_tipo]["pacote"]).union(dados[no_traffic]["pacote"]))
        n_pacotes = len(pacotes)

        resultados_alg[base_tipo] = {}

        # --- FCT ---
        fig, axes = plt.subplots(n_pacotes, 1, figsize=(8, 4 * n_pacotes))
        if n_pacotes == 1:
            axes = [axes]

        for ax, pacote in zip(axes, pacotes):
            base_vals = dados[base_tipo].query("pacote == @pacote")["fct"]
            no_vals = dados[no_traffic].query("pacote == @pacote")["fct"]

            ax.boxplot([base_vals, no_vals], labels=["Com tráfego", "Sem tráfego"])
            ax.set_title(f"FCT - Pacote {pacote/1e6:.1f} MB")
            ax.set_ylabel("FCT (µs)")
            ax.grid(True, axis="y", linestyle="--", alpha=0.7)
            ax.yaxis.set_major_formatter(ScalarFormatter(useOffset=False))
            ax.ticklabel_format(style="plain", axis="y")

            if alg in LOG_SCALE_ALGS:
                ax.set_yscale("log")

            # salvar estatísticas
            resultados_alg[base_tipo][pacote] = {
                "fct_com_trafego": resumo_estatistico(base_vals),
                "fct_sem_trafego": resumo_estatistico(no_vals),
            }

        fig.suptitle("FCT", fontsize=14)
        fig.tight_layout(rect=[0, 0, 1, 0.97])
        plt.savefig(os.path.join(tipo_dir, "fct.png"))
        plt.close()

        # --- Vazão ---
        fig, axes = plt.subplots(n_pacotes, 1, figsize=(8, 4 * n_pacotes))
        if n_pacotes == 1:
            axes = [axes]

        for ax, pacote in zip(axes, pacotes):
            base_vals = dados[base_tipo].query("pacote == @pacote")["vazao"]
            no_vals = dados[no_traffic].query("pacote == @pacote")["vazao"]

            ax.boxplot([base_vals, no_vals], labels=["Com tráfego", "Sem tráfego"])
            ax.set_title(f"Vazão - Pacote {pacote/1e6:.1f} MB")
            ax.set_ylabel("Vazão (MB/s)")
            ax.grid(True, axis="y", linestyle="--", alpha=0.7)
            ax.yaxis.set_major_formatter(ScalarFormatter(useOffset=False))
            ax.ticklabel_format(style="plain", axis="y")

            if alg in LOG_SCALE_ALGS:
                ax.set_yscale("log")

            # adicionar às estatísticas já existentes
            resultados_alg[base_tipo][pacote].update({
                "vazao_com_trafego": resumo_estatistico(base_vals),
                "vazao_sem_trafego": resumo_estatistico(no_vals),
            })

        fig.suptitle("Vazão", fontsize=14)
        fig.tight_layout(rect=[0, 0, 1, 0.97])
        plt.savefig(os.path.join(tipo_dir, "vazao.png"))
        plt.close()

    # --- Salva resumo em TXT ---
    resumo_path = os.path.join(alg_dir, f"resultados_{alg}.txt")
    with open(resumo_path, "w") as f:
        f.write(f"=== Resultados para {alg.upper()} ===\n\n")
        for tipo_exec, pacotes_data in resultados_alg.items():
            f.write(f"[{tipo_exec}]\n")
            for pacote, metricas in pacotes_data.items():
                f.write(f"  Pacote: {pacote/1e6:.1f} MB\n")
                for nome, stats in metricas.items():
                    f.write(f"    {nome.upper()}:\n")
                    for k, v in stats.items():
                        f.write(f"      {k:8}: {v:.4f}\n")
                f.write("\n")
            f.write("\n")

    print(f"  → Resultados salvos em {resumo_path}")

print("\n✅ Gráficos e estatísticas gerados com sucesso!")
