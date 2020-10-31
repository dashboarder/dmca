printf "wAmp: %.2f\n", ((float)mcxt.stat.ddwPagesWrittenCnt * sftl.__geom.vbas_per_page) / (float)sftl.stats.lbas_written
printf "flatten wAmp: %.2f\n", ((float)sftl.stats.lbas_written + sftl.stats.lbas_flatten) / (float)sftl.stats.lbas_written
printf "GC wAmp: %.2f\n", ((float)sftl.stats.lbas_written + sftl.stats.lbas_gc) / (float)sftl.stats.lbas_written
