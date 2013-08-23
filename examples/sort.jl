;; QuickSort in JL.

(define filter (lambda (f lst)
   (if lst
      (if (f (head lst))
         (cons (head lst) (filter f (rest lst)))
         (filter f (rest lst)))
      nil)))

(define merge (lambda (as bs)
   (if as
      (cons (head as) (merge (rest as) bs))
      bs)))

(define sort (lambda (lst)
   (if lst
      (begin
         (define pivot (head lst))
         (define less (filter (lambda (a) (< a pivot)) (rest lst)))
         (define greater (filter (lambda (a) (>= a pivot)) (rest lst)))
         (merge (sort less) (cons pivot (sort greater))))
      nil)))

(print (sort (list 7 3 2 6 9 1 8 4 5)) "\n")

