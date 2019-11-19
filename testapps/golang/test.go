// Example program
package main
import ("fmt")

func myfunc(a int) int {
    return a + 2
}

/* Example main */
func main() {
    var a int = 2
    if a > 1 {
        a = a +1
    }
    a = myfunc(a)
    fmt.Println("Hello!\n")
}


