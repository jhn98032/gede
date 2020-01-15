with Text_IO; use Text_IO;

procedure test is
    j   : integer;
    str : string(1..5);
    c   : character;
    n   : natural;
begin

    str := "hej" & "!!";
    
    n := 10;
    
    c := 'a';
    
    j := 1;
    j := 2**3;
    Put_Line("Hello world!!");
    
    for i in 1.. 10 loop
        j := j + 1;
        Put_Line("loop");
    end loop;

end test;

