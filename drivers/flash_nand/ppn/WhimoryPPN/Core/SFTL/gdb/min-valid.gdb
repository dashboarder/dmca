set $idx = 0
set $max = sftl.__geom.max_sb
set $minv = 65535

while ($idx < $max)
    set $t = sftl.sb[$idx].type
    set $v = sftl.sb[$idx].validLbas
    if (3 == $t)
        if ($v < $minv)
            set $minv = $v
        end
    end
    set $idx = $idx + 1
end

printf "Min valid: %d/%d\n", $minv, sftl.__geom.vbas_per_sb
