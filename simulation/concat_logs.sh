for side in client server; do
  for protocol in timely dctcp; do # dcqcn
    cat saida_${side}_${protocol}_25000.csv \
        saida_${side}_${protocol}_31250.csv \
        saida_${side}_${protocol}_62500.csv \
        saida_${side}_${protocol}_125000.csv \
        saida_${side}_${protocol}_500000.csv \
        > saida_${side}_${protocol}.csv
  done
done