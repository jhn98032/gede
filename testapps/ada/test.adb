--------------------------
-- A simple ada example
--
---------------------------

with Text_IO; use Text_IO;

procedure test is
    j   : integer;
    str : string(1..5);
    c   : character;
    n   : natural;

    function min(a, b : integer) return integer is
    begin
        if a < b then
            return a;
        else
            return b;
        end if;
    end min;

begin

    str := "hej" & "!!";
    
    n := 10;
    
    c := 'a';
    
    j := min(1, 12);
    j := 2**3;
    Put_Line("Hello world!!");
    
    for i in 1.. 10 loop
        j := j + 1;
        Put_Line("loop");
    end loop;

end test;

