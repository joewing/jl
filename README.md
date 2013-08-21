
JL
==============================================================================
This is a small, embeddable LISP-like language.  The intended use is for
configuration files where it is desirable to be able to have complex
configurations (JWM, for example).

This is still a work in progress: there are still more functions to
be implemented and the functionality of existing functions may change.

Syntax
------------------------------------------------------------------------------
Like other LISP languages, JL uses s-expressions.  For example:

  (list 1 2 3)

calls the "list" function, passing 1, 2, and 3 as arguments.

Data Types
------------------------------------------------------------------------------
There are 6 data types:

 1. Numbers (floating point numbers)
 2. Strings
 3. Variables
 4. Lambdas (functions defined within the language)
 5. Lists
 6. Special functions

For comparisons, 0 and nil (the empty list) are considered false and all
other values are considered true.

Functions
------------------------------------------------------------------------------
The following built-in functions are available:

 - <        Test if less than
 - >        Test if greater than
 - <=       Test if less than or equal to
 - >=       Test if greater than or equal to
 - =        Test if equal
 - !=       Test if not equal
 - +        Return the sum of a list
 - -        Subtract
 - *        Return the produce of a list
 - /        Divide
 - %        Modulus
 - and      Logical AND.
 - or       Logical OR.
 - list     Create a list
 - cons     Prepend an item to a list.
 - head     Return the first element of a list
 - rest     Return all but the first element of a list
 - if       Test a condition and evaluate and return the second argument
            if true, otherwise evaluate and return the third argument.
 - define   Insert a binding into the current namespace.
 - lambda   Declare a function.
 - begin    Execute a sequence of functions, return the value of the last.
 - substr   Return a substring of a string.
 - concat   Concatenate strings.
 - number?  Determine if a value is a number.
 - string?  Determine if a value is a string.
 - list?    Determine if a value is a list.
 - null?    Determine if a value is nil.

Examples
------------------------------------------------------------------------------
Here are some example programs.  See the "examples" directory for more.

Return the factorial of a number:
<code><pre>
   (define fact (lambda (n)
      (if n
         (\* (fact (- n 1)) n)
         1)))
   (fact 5)
</pre></code>

Find the nth item of a list:
<code><pre>
   (define nth (lambda (n lst)
      (if (<= n 1)
         (head lst)
         (nth (- n 1) (rest lst)))))
   (nth 2 (list 1 2 3))
</pre></code>

Find nth Fibonacci number:
<code><pre>
   (define fib (lambda (n)
      (if (> n 1)
         (+ (fib (- n 1)) (fib (- n 2)))
         1)))
   (fib 10)
</pre></code>

The map function:
<code><pre>
   (define map (lambda (f lst)
      (if lst
         (cons (f (head lst)) (map f (rest lst)))
         (list))))
   (map (lambda (x) (+ x 1)) (list 1 2 3 4))
</pre></code>

The foldl function:
<code><pre>
   (define foldl (lambda (f i lst)
      (if lst
         (foldl f (f i (head lst)) (rest lst))
         i)))
   (foldl (lambda (a b) (+ a b)) 0 (list 1 2 3 4))
</pre></code>

The reverse function implemented in terms of foldl:
<code><pre>
   (define reverse (lambda (lst) (foldl (lambda (a b) (cons b a)) (list) lst)))
   (reverse (list 1 2 3 4))
</pre></code>

License
------------------------------------------------------------------------------
JL uses the BSD 2-clause license.  See LICENSE for more information.

