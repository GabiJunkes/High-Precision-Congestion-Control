import os
import re
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

def formatar_bytes_para_mb(bytes_val):
    """Converte bytes para MB para deixar o título mais bonito no TCC."""
    val = bytes_val / 10**6
    val = int(round(val, 1 - len(str(int(val)))))
    return f"{val} MB"

# Configuração do diretório
BASE_DIR = "../../simulation/"

folder_pattern = re.compile(r"log_output_inter_(\d+)")

file_pattern = re.compile(r"saida_client_([a-zA-Z0-9]+)_(\d+)_7812\.csv")

data_list = []

print(f"Buscando logs em: {BASE_DIR}")

for folder_name in os.listdir(BASE_DIR):
    folder_path = os.path.join(BASE_DIR, folder_name)
    
    if not os.path.isdir(folder_path):
        continue
        
    folder_match = folder_pattern.match(folder_name)
    if folder_match:
        traffic_pct = int(folder_match.group(1))
        
        for file_name in os.listdir(folder_path):
            file_match = file_pattern.match(file_name)
            
            if file_match:
                alg = file_match.group(1)
                packet_size = int(file_match.group(2))
                file_path = os.path.join(folder_path, file_name)
                
                # Lê o arquivo e extrai as linhas que começam com "C"
                with open(file_path, 'r') as f:
                    for line in f:
                        line = line.strip()
                        if line.startswith('C;'):
                            parts = line.split(';')
                            # Formato esperado: C; process_time; size; fct; throughput
                            if len(parts) >= 5:
                                process_time = float(parts[1].strip())
                                size = int(parts[2].strip())
                                fct = float(parts[3].strip())
                                throughput = float(parts[4].strip())
                                
                                data_list.append({
                                    'Tráfego (%)': traffic_pct,
                                    'Algoritmo': alg.upper() if alg != 'hp' else 'HPCC',
                                    'Tamanho do Pacote': packet_size,
                                    'FCT (us)': fct,
                                    'Vazão (MB/s)': throughput
                                })

# 2. Criação do DataFrame
df = pd.DataFrame(data_list)

if df.empty:
    print("Nenhum dado encontrado. Verifique o BASE_DIR e os nomes dos arquivos.")
    exit()

print(f"Foram carregadas {len(df)} amostras de logs do tipo 'C'. Gerando gráficos...")

sns.set_theme(style="whitegrid")

alg_order = [
    "DCQCN",
    "DCTCP",
    "HPCC",
    "TIMELY",
]

algoritmos_unicos = sorted(df['Algoritmo'].unique())

# Gera a paleta e o dicionário de cores
cores_seaborn = sns.color_palette("tab10", len(algoritmos_unicos))
paleta_algoritmos = dict(zip(algoritmos_unicos, cores_seaborn))
# --------------------------------------------------


for traffic in df['Tráfego (%)'].unique():
    for pkt_size in df['Tamanho do Pacote'].unique():
        
        # Filtra os dados para o tráfego e tamanho de pacote específicos
        subset = df[(df['Tráfego (%)'] == traffic) & (df['Tamanho do Pacote'] == pkt_size)]
        
        if subset.empty:
            continue

        tamanho_mb = formatar_bytes_para_mb(pkt_size)
            
        # ==========================================
        # Gráfico 1: Throughput
        # ==========================================
        plt.figure(figsize=(8, 6))
        ax1 = sns.boxplot(
            data=subset, 
            x='Algoritmo', 
            y='Vazão (MB/s)',
            hue='Algoritmo',
            order=alg_order,
            palette=paleta_algoritmos,
            legend=False
        )
        plt.title(f'Vazão\nTráfego de Fundo: {traffic}% | Pacote: {tamanho_mb}', pad=15)
        plt.ylabel('Vazão (MB/s)', fontsize=12)
        plt.xlabel('Algoritmo', fontsize=12)
        
        # Salva a imagem com alta resolução (ideal para o documento do TCC)
        out_filename_tp = f"boxplot_vazao_{traffic}_{pkt_size}.png"
        plt.savefig(out_filename_tp, dpi=300, bbox_inches='tight')
        plt.close()

        # ==========================================
        # Gráfico 2: FCT (Flow Completion Time)
        # ==========================================
        plt.figure(figsize=(8, 6))
        ax2 = sns.boxplot(
            data=subset, 
            x='Algoritmo', 
            y='FCT (us)', 
            hue='Algoritmo',
            order=alg_order,
            palette=paleta_algoritmos,
            legend=False
        )
        plt.title(f'Tempo de Conclusão do Fluxo (FCT)\nTráfego de Fundo: {traffic}% | Pacote: {tamanho_mb}', pad=15)
        plt.ylabel('FCT (\u03bcs)', fontsize=12)
        plt.xlabel('Algoritmo', fontsize=12)
        
        out_filename_fct = f"boxplot_fct_{traffic}_{pkt_size}.png"
        plt.savefig(out_filename_fct, dpi=300, bbox_inches='tight')
        plt.close()

print(f"Gráficos gerados com sucesso!")