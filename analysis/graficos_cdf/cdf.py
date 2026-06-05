import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import glob
import os
import re

# Configurações de estilo para o TCC
plt.style.use('seaborn-v0_8-whitegrid')
plt.rcParams.update({
    'font.size': 12, 
    'font.family': 'serif',
    'axes.labelsize': 14,
    'legend.fontsize': 12
})

def formatar_bytes_para_mb(bytes_val):
    """Converte bytes para MB para deixar o título mais bonito no TCC."""
    val = bytes_val / 10**6
    val = int(round(val, 1 - len(str(int(val)))))
    return f"{val} MB"

def main():
    dados_p = []

    # O asterisco (*) no meio do caminho pega qualquer pasta que comece com 'log_output_inter_'
    caminho_busca = '../../simulation/log_output_inter_*/saida_client_*.csv'
    arquivos = glob.glob(caminho_busca)
    
    if not arquivos:
        print(f"Nenhum arquivo encontrado no caminho: {caminho_busca}")
        return

    print(f"Foram encontrados {len(arquivos)} arquivos de log. Processando...")

    for arquivo in arquivos:
        pasta = os.path.dirname(arquivo)
        
        match_trafego = re.search(r'inter_(\d+)', pasta)
        trafego = int(match_trafego.group(1)) if match_trafego else 0
        
        nome_base = os.path.basename(arquivo).replace('.csv', '')
        partes = nome_base.split('_')
        
        if len(partes) >= 5:
            algoritmo = partes[2].upper() if partes[2].upper() != 'HP' else 'HPCC'
            tamanho_pacote = int(partes[3])
            
            try:
                df = pd.read_csv(arquivo, sep=';', header=None, 
                                 names=['Tipo', 'ProcessTime', 'Size', 'LockedEvents', 'Starvation'],
                                 engine='python')
                
                df['Tipo'] = df['Tipo'].astype(str).str.strip()
                df_p = df[df['Tipo'] == 'P'].copy()
                
                df_p['Algoritmo'] = algoritmo
                df_p['TamanhoPacote'] = tamanho_pacote
                df_p['Trafego'] = trafego
                
                dados_p.append(df_p)
            except Exception as e:
                print(f"Erro ao processar {arquivo}: {e}")

    if not dados_p:
        print("Nenhum dado do tipo 'P' foi extraído dos logs.")
        return

    df_final = pd.concat(dados_p, ignore_index=True)
    df_final['LockedEvents'] = pd.to_numeric(df_final['LockedEvents'])

    # --- DEFINIÇÃO DA PALETA DE CORES E ORDEM FIXAS ---
    # Identifica todos os algoritmos e os ordena alfabeticamente
    algoritmos_unicos = sorted(df_final['Algoritmo'].unique())
    
    # Gera a paleta e o dicionário de cores
    cores_seaborn = sns.color_palette("tab10", len(algoritmos_unicos))
    paleta_algoritmos = dict(zip(algoritmos_unicos, cores_seaborn))
    # --------------------------------------------------

    trafegos = df_final['Trafego'].unique()
    tamanhos = df_final['TamanhoPacote'].unique()

    print("\nGerando gráficos...")

    for trafego in sorted(trafegos):
        for tamanho in sorted(tamanhos):
            df_plot = df_final[(df_final['Trafego'] == trafego) & (df_final['TamanhoPacote'] == tamanho)]
            
            if df_plot.empty:
                continue

            plt.figure(figsize=(8, 5))
            
            # Plota a CDF com a paleta E a ordem da legenda fixas
            sns.ecdfplot(
                data=df_plot, 
                x='LockedEvents', 
                hue='Algoritmo', 
                palette=paleta_algoritmos, 
                hue_order=algoritmos_unicos, # <- GARANTE A ORDEM DA LEGENDA AQUI
                linewidth=2.5
            )
            
            tamanho_mb = formatar_bytes_para_mb(tamanho)
            plt.title(f'CDF de Eventos de Lock\nTráfego de Fundo: {trafego}% | Pacote: {tamanho_mb}', pad=15)
            plt.xlabel('Número de Locked Events')
            plt.ylabel('Probabilidade Acumulada (CDF)')
            
            plt.xlim(left=0)
            plt.ylim(0, 1.05)
            plt.grid(True, linestyle='--', alpha=0.7)
            plt.tight_layout()
            
            nome_imagem = f'cdf_trafego_{trafego}_pacote_{tamanho}.png'
            plt.savefig(nome_imagem, dpi=300, bbox_inches='tight')
            plt.close()
            
            print(f"[+] Salvo: {nome_imagem}")

    print("\nProcesso concluído! Gráficos prontos para o TCC.")

if __name__ == "__main__":
    main()