Declare Function func(ByVal arg1 As Double) As Double

/' Multiline comment. Row1
   Row2.
'/
rem Single line program
Dim C aS InteGer
'Start of program
Dim A As Integer
Dim D As Double
CLS


sub example_sub()
    print "Hello"
end sub

PRINT "Looping"

A=0
DO
D=func(1.2)
A=A+1
? "A =";A
Loop Until A=5

PRINT "Loop done and C =";A
SLEEP
'End of program

End

Function func(ByVal arg1 As Double) As Double
   Dim res As Double
   res = arg1 * 1.123
   arg1 = arg1 * 3.45
   return res
End Function

Function func2(ByVal arg1 As Double) As Double
   Dim res As Double
   res = arg1 * 1.123
   arg1 = arg1 * 3.45
   print res
   return res
End Function


Function func3(ByVal arg1 As Double) As Double
   Dim res As Double
   res = arg1 * 1.123
   arg1 = arg1 * 3.45
   return res
End Function
