set $idx = 0
set $max = sftl.__geom.max_sb

while ($idx < $max)
    set $t = sftl.sb[$idx].type
    if (3 == $t)
        set $a = sftl.sb[$idx].validLbas
        set $b = sftl.dbg.validSums[$idx]
        if ($a != $b)
            printf "%d: %d <-> %d; %2.5f\n", $idx, $a, $b, ((float)$a)/(float)$b
        end
    end
    set $idx = $idx + 1
end

