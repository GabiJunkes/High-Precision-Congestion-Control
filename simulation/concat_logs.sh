#!/usr/bin/env bash
# concat_saida.sh
# Junta arquivos do tipo saida_${type}_${alg}_*_*.csv da pasta log_output/
# e gera um único arquivo por tipo e algoritmo na pasta log_output_formated/

input_dir="outputs/raw/intra_no_traffic"
output_dir="outputs/formated/intra_no_traffic"

# Cria pasta de saída se não existir
mkdir -p "$output_dir"

shopt -s nullglob

for file in "$input_dir"/saida_*_*_*_*.csv; do
    # Extrai apenas o nome base (sem o caminho)
    base=$(basename "$file")

    # Extrai os campos do nome: saida_type_alg_...
    IFS='_' read -r prefix type alg rest <<< "$base"

    # Nome de saída
    output="${output_dir}/saida_${type}_${alg}.csv"

    echo "Adicionando $base em $output"
    cat "$file" >> "$output"
done