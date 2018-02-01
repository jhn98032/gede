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
