set $sb = 0
set $bank = 0
set $sb_max = mcxt.dev.blocks_per_cau
set $bank_max = mcxt.dev.num_of_banks
set $good = 0
set $bad = 0

while ($sb < $sb_max)
    set $bank = 0
    set $base = mcxt.r_gen.bytes_per_vbn_bitmap * $sb

    while ($bank < $bank_max)
        if (mcxt.v2p_bitmap[$base + ($bank >> 3)] & (1 << ($bank % 8)))
            set $good = $good + 1
        else
            set $bad = $bad + 1
        end

        set $bank = $bank + 1
    end

    set $sb = $sb + 1
end

printf "Good blocks: %d\n", $good
printf "Bad blocks: %d\n", $bad
printf "Total blocks: %d\n", ($good + $bad)
printf "Total FTL blocks: %d\n", (sftl.__geom.max_sb * sftl.__geom.num_banks)
printf "Bad block FTL percentage: %.3f\n", ((float)$bad * 100.0 / (sftl.__geom.max_sb * sftl.__geom.num_banks))
