set $idx = 0
set $max = sftl.__geom.max_sb
set $limit = 1000

while ($idx < $max)
    set $r = sftl.sb[$idx].reads
    if ($r >= $limit)
        printf "[%d]: %d\n", $idx, $r
    end
    set $idx = $idx + 1
end

