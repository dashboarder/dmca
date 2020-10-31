set logging redirect on
set logging overwrite on
set logging file ~/userSeq.txt
set logging on

set $idx = 0
set $max = sftl.__geom.max_sb

while ($idx < $max)
    set $t = sftl.sb[$idx].type
    set $v = 0
    if (3 == $t)
        set $v = sftl.sb[$idx].validLbas
        printf "sb:%5d    erases:%5d    static:%d    userSeq min:%9d    max:%9d    avg:%9d    length:%5d\n", $idx, sftl.sb[$idx].erases, (sftl.sb[$idx].num_btoc_vbas_AND_staticFlag & 1), sftl.sb_userSeq[$idx].min, sftl.sb_userSeq[$idx].max, sftl.sb_userSeq[$idx].avg, sftl.sb_userSeq[$idx].count

    end
    set $idx = $idx + 1
end

set logging off
printf "Saved to ~/userSeq.txt; please analyze\n"
