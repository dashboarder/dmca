set $idx = 0
set $max = sftl.__geom.max_sb

while ($idx < $max)
    set $t = sftl.sb[$idx].type
    set $v = sftl.sb[$idx].validLbas
    if ((0 == $v) && (1 != $t))
        printf "[%d]: %d\n", $idx, $t
    end
    set $idx = $idx + 1
end

