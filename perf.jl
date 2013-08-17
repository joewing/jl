
(define perf1 (lambda (n) (if n (+ n (perf1 (- n 1))) 0)))
(define perf (lambda (n) (if n (+ (perf1 n) (perf (- n 1))) 0)))

(perf 2048)
