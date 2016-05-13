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
   (if (null? lst) 0 (+ 1 (length (rest lst))))))

(assert (= (length (list 1 2 3 4)) 4))
(assert (= (length (list)) 0))

(define nth (lambda (n lst)
   (if (<= n 1) (head lst)
      (nth (- n 1) (rest lst)))))

(assert (= (nth 2 (list 5 4 3 2 1)) 4))
(assert (= (nth 1 (list 5 4 3 2 1)) 5))

(define map (lambda (f lst)
   (if (null? lst) nil (cons (f (head lst)) (map f (rest lst))))))

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
   (add 1)))
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

(assert (not (and 0 (assert 0))))
(assert (= 1 (or 1)))
(assert (= 1 (not 0)))

(define strlen (lambda (str)
   (define helper (lambda (i) (if (substr str i 1) (helper (+ i 1)) i)))
   (helper 0)))

(assert (= (strlen "asdf") 4))

(assert (= 1 (number? 5)))
(assert (= nil (number? nil)))
(assert (= nil (number? "test")))
(assert (= nil (number? (list 1 2 3))))
(assert (= 1 (number? (head (list 1 2 3)))))

(assert (= 1 (string? "asfd")))
(assert (= nil (string? nil)))
(assert (= nil (string? 5)))
(assert (= nil (string? (list 1 2 3))))
(assert (= 1 (string? (head (list "asdf" 2 3)))))

(assert (= 1 (list? (list 1 2))))
(assert (= nil (list? nil)))
(assert (= nil (list? 5)))
(assert (= nil (list? "test")))
(assert (= 1 (list? (head (list (list 1 2) 2 3)))))

(assert (< "a" "b"))
(assert (= "asdf" "asdf"))
(assert (<= "a" "b"))
(assert (<= "a" "a"))
(assert (!= "ab" "ac"))
(assert (> "b" "a"))
(assert (>= "b" "a"))
(assert (>= "a" "a"))

(assert (= ((lambda (a b) (+ a b 1)) 2 3) 6))

; Test concat and whitespace before closing ')'.
(define repeat
    (lambda (str i)
        (if (= i 1)
            str
            (if (> i 0)
                (concat (repeat str (- i 1)) "," str)
                ""
            )
        )
    )
)
(assert (= "0123456789,0123456789,0123456789" (repeat "0123456789" 3)))

(print "\ndone\n")

