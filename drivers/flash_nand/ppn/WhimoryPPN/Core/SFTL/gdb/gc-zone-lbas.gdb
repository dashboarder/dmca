set $idx = 0
set $max = sftl.gc.data.curZoneSize

while ($idx < $max)
    printf "%d: %d\n", $idx, sftl.gc.data.meta[$idx].__f1
    set $idx = $idx + 1
end
