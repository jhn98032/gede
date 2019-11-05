! comment

Subroutine sfunc1()
    print *, "Hello World again!"
end Subroutine sfunc1

Program hello
    integer :: y
    real :: x
    x = 1
    x = 2
    y = 2
    x = 3
    y = 3
    print *, "Hello World!"
    call sfunc1()
end program hello




