set logging redirect on
set logging overwrite on
set logging file ~/valids.txt
set logging on

set $idx = 0
set $max = sftl.__geom.max_sb
set $vbas_per_sb = sftl.__geom.vbas_per_sb

set $bank_max = mcxt.dev.num_of_banks

while ($idx < $max)
    set $bank = 0
    set $base = mcxt.r_gen.bytes_per_vbn_bitmap * $idx

    set $good = 0
    set $bad = 0
    while ($bank < $bank_max)
        if (mcxt.v2p_bitmap[$base + ($bank >> 3)] & (1 << ($bank % 8)))
            set $good = $good + 1
        else
            set $bad = $bad + 1
        end

        set $bank = $bank + 1
    end

    set $t = sftl.sb[$idx].type
    set $v = 0
    if (3 == $t)
        set $v = sftl.sb[$idx].validLbas
    end
    printf "%d:%d/%d,%.2f\n", $idx, $v, ($vbas_per_sb * $good) / $bank_max, (100 * ((float)$v * $bank_max) / $good / $vbas_per_sb)
    set $idx = $idx + 1
end

set logging off
printf "Saved to ~/valids.txt; please sort and graph\n"
