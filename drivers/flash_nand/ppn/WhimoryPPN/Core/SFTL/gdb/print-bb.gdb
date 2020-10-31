set $sb = 0
set $bank = 0
set $sb_max = mcxt.dev.blocks_per_cau
set $bank_max = mcxt.dev.num_of_banks

while ($sb < $sb_max)
    set $bank = 0
    set $base = mcxt.r_gen.bytes_per_vbn_bitmap * $sb

    printf "sb:0x%04x |", $sb
    while ($bank < $bank_max)
        if (mcxt.v2p_bitmap[$base + ($bank >> 3)] & (1 << ($bank % 8)))
            if (mcxt.v2p_scrub_bitmap[$base + ($bank >> 3)] & (1 << ($bank % 8)))
                printf "s"
            else
                printf " "
	    end
        else
            printf "x"
        end

        set $bank = $bank + 1
    end
    printf "|\n"

    set $sb = $sb + 1
end

