import os
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter
from pathlib import Path

# Caminhos base
BASE_DIR = "../simulation/outputs/formated"
OUTPUT_DIR = "graficos"

TIPOS = ["intra_30_traffic", "intra_no_traffic", "inter_30_traffic", "inter_no_traffic"]

# opcional: aplicar escala logarítmica em alguns algoritmos
LOG_SCALE_ALGS = []  # exemplo: ["hpcc", "timely"]

os.makedirs(OUTPUT_DIR, exist_ok=True)

def formatar_para_sci(valor):
        """Converte um float para o formato LaTeX 'NUM\sci{eEXP}'"""
        # Formata o número, ex: "9.38e+10"
        string_cientifica = f"{valor:.2e}"
        # Separa o número do expoente, ex: ["9.38", "+10"]
        partes = string_cientifica.split('e')
        # Remonta no formato desejado: "9.38\sci{e+10}"
        return f"{partes[0]}\\sci{{e{partes[1]}}}"

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
                    vazao = float(partes[4]) * 8000000
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

    for base in ["intra", "inter"]:
        no_traffic = base + "_no_traffic"
        base_tipo = base + "_30_traffic"
        if base_tipo not in dados or no_traffic not in dados:
            continue

        tipo_dir = os.path.join(alg_dir, base)
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
            
            ax.ticklabel_format(style='sci', axis='y', scilimits=(0,0))

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
    # with open(resumo_path, "w") as f:
    #     f.write(f"=== Resultados para {alg.upper()} ===\n\n")
    #     for tipo_exec, pacotes_data in resultados_alg.items():
    #         f.write(f"[{tipo_exec}]\n")
    #         for pacote, metricas in pacotes_data.items():
    #             f.write(f"  Pacote: {pacote/1e6:.1f} MB\n")
    #             for nome, stats in metricas.items():
    #                 f.write(f"    {nome.upper()}:\n")
    #                 for k, v in stats.items():
    #                     f.write(f"      {k:8}: {v:.4f}\n")
    #             f.write("\n")
    #         f.write("\n")

    
    with open(resumo_path, "w") as f:
        f.write(f"% Resultados em LaTeX para {alg.upper()}\n")
        f.write(f"% Lembre-se de adicionar os pacotes no seu documento principal:\n")
        f.write(f"% \\usepackage{{booktabs}} (para top/mid/bottomrule)\n")
        f.write(f"% \\usepackage{{float}} (para a opção [H])\n")
        f.write(f"% \\usepackage{{glossaries}} (para \\gls)\n\n")

        # Itera sobre cada tipo de execução (ex: "intra_30_traffic")
        for tipo_exec, pacotes_data in resultados_alg.items():
            
            # Determina o nome base (intra ou inter) para o caption
            base_nome = "intra" if "intra" in tipo_exec else "inter"
            
            alg = "hpcc" if "hp" == alg else alg

            f.write(f"\\begin{{table}}[H]\n")
            f.write(f"\\centering\n")
            f.write(f"\\caption{{Resultados {base_nome} \\textit{{pod}} para o protocolo \\gls{{{alg}}}}}\n")
            f.write(f"\\label{{tab:{alg}_{base_nome}}}\n")
            
            # Alterado para 'lrrrrr' (6 colunas)
            f.write(f"\\begin{{tabular}}{{lrrrrr}}\n") 
            f.write(f"\\toprule\n")
            
            # Cabeçalho atualizado (sem coluna Pacote)
            f.write(f"\\textbf{{Métrica}} & \\textbf{{Média}} & \\textbf{{Mediana}} & \\textbf{{Desvio}} & \\textbf{{Mínimo}} & \\textbf{{Máximo}} \\\\\n")
            f.write(f"\\midrule\n")

            # Itera sobre os pacotes (ordenados por tamanho)
            for i, pacote in enumerate(sorted(pacotes_data.keys())):
                
                # Adiciona \midrule de separação se não for o primeiro pacote
                if i > 0:
                    f.write(f"\\midrule\n")

                metricas = pacotes_data[pacote]
                pacote_mb = f"{pacote/1e6:.1f}"

                # --- Adiciona o Subtítulo do Pacote ---
                f.write(f"\\multicolumn{{6}}{{l}}{{\\textbf{{Pacote {pacote_mb} MB}}}} \\\\\n")
                f.write(f"\\cmidrule(r){{1-6}}\n")

                # --- Linhas de FCT (sem a coluna {pacote_mb}) ---
                stats_fct_com = metricas["fct_com_trafego"]
                f.write(f"\\gls{{fct}} c/ tráfego & "
                        f"{stats_fct_com['media']:.2f}$\\mu$s & {stats_fct_com['mediana']:.2f}$\\mu$s & "
                        f"{stats_fct_com['desvio']:.2f}$\\mu$s & {stats_fct_com['minimo']:.2f}$\\mu$s & "
                        f"{stats_fct_com['maximo']:.2f}$\\mu$s \\\\\n")

                stats_fct_sem = metricas["fct_sem_trafego"]
                f.write(f"\\gls{{fct}} s/ tráfego & "
                        f"{stats_fct_sem['media']:.2f}$\\mu$s & {stats_fct_sem['mediana']:.2f}$\\mu$s & "
                        f"{stats_fct_sem['desvio']:.2f}$\\mu$s & {stats_fct_sem['minimo']:.2f}$\\mu$s & "
                        f"{stats_fct_sem['maximo']:.2f}$\\mu$s \\\\\n")

                # --- Linhas de Vazão (sem a coluna {pacote_mb}) ---
                stats_vazao_com = metricas["vazao_com_trafego"]
                f.write(f"Vazão c/ tráfego & "
                        f"{formatar_para_sci(stats_vazao_com['media'])} & {formatar_para_sci(stats_vazao_com['mediana'])} & "
                        f"{formatar_para_sci(stats_vazao_com['desvio'])} & {formatar_para_sci(stats_vazao_com['minimo'])} & "
                        f"{formatar_para_sci(stats_vazao_com['maximo'])} \\\\\n")

                stats_vazao_sem = metricas["vazao_sem_trafego"]
                f.write(f"Vazão s/ tráfego & "
                        f"{formatar_para_sci(stats_vazao_sem['media'])} & {formatar_para_sci(stats_vazao_sem['mediana'])} & "
                        f"{formatar_para_sci(stats_vazao_sem['desvio'])} & {formatar_para_sci(stats_vazao_sem['minimo'])} & "
                        f"{formatar_para_sci(stats_vazao_sem['maximo'])} \\\\\n")

            # --- Fim da Tabela ---
            f.write(f"\\bottomrule\n")
            f.write(f"\\end{{tabular}}\n")
            f.write(f"\\end{{table}}\n\n")

    print(f"  → Resultados salvos em {resumo_path}")

print("\n✅ Gráficos e estatísticas gerados com sucesso!")
