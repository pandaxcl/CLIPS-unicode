;;; vim: expandtab autoindent
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(deffunction vim ($?msg)
    (printout t "]51;" (implode$ ?msg) "" crlf))
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;; (vim [ "call", "Tapi_hello", {
;;;     "哈哈": "haha",
;;;     "呵呵": (str-cat hehe),
;;; }])

;;; :function! Tapi_hello(bufnr, json)
;;; :	echomsg "来自Tapi_hello的消息：" a:json
;;; :endfunction

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
