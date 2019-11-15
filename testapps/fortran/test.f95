 ! comment

#include "inc.f95"

#ifdef NEVER
    integer :: n
#endif

Subroutine sfunc1()
    print *, "Hello \\ World again!\\"
end Subroutine sfunc1

Program hello
    integer :: y
    real :: x
    x = 1.1
    x = 2.3
    y = 2
    x = 3.4
    y = 3
    print *, "Hello World!"
    call sfunc1()
end program hello




