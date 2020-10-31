set $unknown = 0
set $erased = 0
set $pending = 0
set $ebu = 0
set $data = 0
set $data_cur = 0
set $data_gc = 0
set $data_pending_gc = 0
set $cxt = 0
set $dead = 0

set $idx = 0
set $max = sftl.__geom.max_sb
while ($idx < $max)
    set $t = sftl.sb[$idx].type
    if (1 == $t)
        set $erased = $erased + 1
    else
    if (2 == $t)
        set $pending = $pending + 1
    else
    if (3 == $t)
        set $data = $data + 1
    else
    if (4 == $t)
        set $data_cur = $data_cur + 1
    else
    if (5 == $t)
        set $data_gc = $data_gc + 1
    else
    if (6 == $t)
        set $data_pending_gc = $data_pending_gc + 1
    else
    if (7 == $t)
        set $cxt = $cxt + 1
    else
    if (8 == $t)
        set $dead = $dead + 1
    else
    if (9 == $t)
        set $ebu = $ebu + 1
    else
        printf "Unknown block type: sftl.sb[%d].type = %d\n", $idx, $t
        set $unknown = $unknown + 1
    end
    end
    end
    end
    end
    end
    end
    end
    end

    set $idx = $idx + 1
end

printf "unknown: %d\n", $unknown
printf "erased: %d\n", $erased
printf "erase-before-use: %d\n", $ebu
printf "pending: %d\n", $pending
printf "data: %d\n", $data
printf "data_cur: %d\n", $data_cur
printf "data_gc: %d\n", $data_gc
printf "data_pending_gc: %d\n", $data_pending_gc
printf "cxt: %d\n", $cxt
printf "dead: %d\n", $dead
