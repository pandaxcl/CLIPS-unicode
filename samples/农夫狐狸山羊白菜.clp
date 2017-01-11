;;; 1. 农夫会划船
;;; 2. 狐狸吃山羊，没有农夫在场
;;; 3. 山羊吃白菜，没有农夫在场
;;; 4. 农夫狐狸山羊白菜在河边
;;; 5. 农夫想把狐狸山羊白菜运到河对岸，且不能发生任何意外
;;; 6. 船一次最多只能运送两个对象

(deftemplate 状态
	(slot i (type INTEGER)(default 0))
	(slot p (type FACT-ADDRESS SYMBOL)(default nil))
	(slot 船 (allowed-values 这边 那边))
	(slot 农夫 (allowed-values 这边 那边))
	(slot 狐狸 (allowed-values 这边 那边))
	(slot 山羊 (allowed-values 这边 那边))
	(slot 白菜 (allowed-values 这边 那边)))

(deffacts 问题描述
	(对象 农夫)
	(对象 狐狸)
	(对象 山羊)
	(对象 白菜)
	(对象 船)
	
	(吃掉 狐狸 山羊)
	(吃掉 山羊 白菜)
	
	(划船 农夫)
	
	(对岸 这边 那边)
	(对岸 那边 这边)

	(状态 (i 0)
		  (船 这边)
		  (农夫 这边)
		  (狐狸 这边)
		  (山羊 这边)
		  (白菜 这边))
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defrule 目标已达成
	(declare (salience 100))
	(状态 (船 那边)
		  (农夫 那边)
		  (狐狸 那边)
		  (山羊 那边)
		  (白菜 那边))
=>
	(printout t "【目标已达成】" crlf)
)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defrule 农夫划船-不运送任何物体
	?f <- (状态 (i    ?i)
		        (船   ?A)
		        (农夫 ?A))
	(对岸 ?A ?B)
=>
	(duplicate ?f (i (+ ?i 1))
				  (p ?f)
				  (船   ?B)
				  (农夫 ?B))
)
(defrule 农夫划船-运送狐狸到对岸
	?f <- (状态 (i    ?i)
		        (船   ?A)
		        (狐狸 ?A)
		        (农夫 ?A))
	(对岸 ?A ?B)
=>
	(duplicate ?f (i (+ ?i 1))
				  (p ?f)
				  (船   ?B)
				  (狐狸 ?B)
				  (农夫 ?B))
)
(defrule 农夫划船-运送山羊到对岸
	?f <- (状态 (i    ?i)
		        (船   ?A)
		        (山羊 ?A)
		        (农夫 ?A))
	(对岸 ?A ?B)
=>
	(duplicate ?f (i (+ ?i 1))
				  (p ?f)
				  (船   ?B)
				  (山羊 ?B)
				  (农夫 ?B))
)
(defrule 农夫划船-运送白菜到对岸
	?f <- (状态 (i    ?i)
		        (船   ?A)
		        (白菜 ?A)
		        (农夫 ?A))
	(对岸 ?A ?B)
=>
	(duplicate ?f (i (+ ?i 1))
				  (p ?f)
				  (船   ?B)
				  (白菜 ?B)
				  (农夫 ?B))
)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defrule 不可以出现循环
	(declare (salience 100))
	(状态 (i    ?i)
		  (船   ?a)
		  (农夫 ?b)
		  (狐狸 ?c)
		  (山羊 ?d)
		  (白菜 ?e))
	?f <- (状态 (i    ?j&:(> ?j ?i))
		        (船   ?a)
		        (农夫 ?b)
		        (狐狸 ?c)
		        (山羊 ?d)
		        (白菜 ?e))
	
=>
	(retract ?f)
)

(watch rules)
(watch facts)
(reset)
(facts)
(run)
(exit)
