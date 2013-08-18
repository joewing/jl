;;; Test suite for JL

; Assert that an expression is true.  Prints a dot if true, or FAIL if false.
(define assert (lambda (tst) (if tst (print ".") (print "\nFAIL\n"))))

(define fib (lambda (n)
   (if (> n 1)    ; Comment in the middle of a line
      (+ (fib (- n 1)) (fib (- n 2)))
      1)))

(assert (= (fib 4) 5))
(assert (= (fib 0) 1))

(define fact (lambda (n)
   (if (> n 0) (* (fact (- n 1)) n) 1)))

(assert (= (fact 5) 120))

(define length (lambda (lst)
   (if lst (+ 1 (length (rest lst))) 0)))

(assert (= (length (list 1 2 3 4)) 4))
(assert (= (length (list)) 0))

(define nth (lambda (n lst)
   (if (<= n 1) (head lst)
      (nth (- n 1) (rest lst)))))

(assert (= (nth 2 (list 5 4 3 2 1)) 4))
(assert (= (nth 1 (list 5 4 3 2 1)) 5))

(define map (lambda (f lst)
   (if lst
      (cons (f (head lst)) (map f (rest lst)))
      (list))))

(define foldl (lambda (f i lst)
   (if lst (foldl f (f i (head lst)) (rest lst)) i)))

(assert (= (foldl (lambda (a b) (+ a b)) 1 (list 2 3 4)) 10))

(assert (= (foldl (lambda (a b) (+ a b)) 0
                  (map (lambda (x) (+ x 1)) (list 0 1 2 3))) 10))

(define reverse (lambda (lst) (foldl (lambda (a b) (cons b a)) (list) lst)))

(define make-increment (lambda (i) (lambda (x) (+ x i))))
(define add-five (make-increment 5))
(assert (= (add-five 2) 7))

(define add (lambda (x y) (+ x y)))
(define do-define (lambda (x)
   (define add (lambda (y) (- x y)))
   (bah 1)))
(assert (= (do-define 3) 2))
(assert (= (add 1 2) 3))

(define x 1)
(define x 2)
(assert (= x 2))

(assert (= (begin 1 2) 2))

(define y 1)
(assert (= y 1))
(begin
   (assert (= y 1))
   (define y 2)
   (assert (= y 2)))
(assert (= y 1))

(assert (= 0 (and 0 (assert 0))))
(assert (= 1 (or 1)))

(print "\ndone\n")
