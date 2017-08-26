;------------------------------------------------------------------------------
; syn
; Copyright (c) 2013-2017, Joshua Scoggins and Contributors
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;     * Redistributions of source code must retain the above copyright
;       notice, this list of conditions and the following disclaimer.
;     * Redistributions in binary form must reproduce the above copyright
;       notice, this list of conditions and the following disclaimer in the
;       documentation and/or other materials provided with the distribution.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
; ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
; ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;------------------------------------------------------------------------------
; test_maya.clp - Test the extra maya user defined functions
;------------------------------------------------------------------------------
(batch* lib/cortex.clp)
(batch* lib/test.clp)
(defmodule MAIN
           (import cortex
                   ?ALL)
           (import test
                   ?ALL))
(deffacts MAIN::boost-test-cases
          (testcase (id boost-has-prefix0)
                    (description "does has-prefix work?"))
          (testcase (id boost-has-suffix0)
                    (description "does has-suffix work?"))
          (testcase (id boost-string-trim0)
                    (description "does string-trim work?"))
          (testcase (id boost-string-trim-front0)
                    (description "does string-trim-front work?"))
          (testcase (id boost-string-trim-back0)
                    (description "does string-trim-back work?"))
          (testcase (id boost-fs-path-exists0)
                    (description "does path-exists work on a file?"))
          (testcase (id boost-fs-path-exists1)
                    (description "does path-exists work on a directory?"))
          (testcase (id boost-fs-directoryp0)
                    (description "does directoryp work?"))
          (testcase (id boost-fs-regular-filep0)
                    (description "does regular-filep work?"))
          (testcase (id boost-clamp0)
                    (description "test the range clamp"))
          (testcase-assertion (parent boost-fs-path-exists0)
                              (expected TRUE)
                              (actual-value (path-exists "lib/cortex.clp")))
          (testcase-assertion (parent boost-fs-path-exists1)
                              (expected TRUE)
                              (actual-value (path-exists "lib/")))
          (testcase-assertion (parent boost-fs-directoryp0)
                              (expected TRUE)
                              (actual-value (directoryp "lib/")))
          (testcase-assertion (parent boost-fs-regular-filep0)
                              (expected TRUE)
                              (actual-value (regular-filep "lib/cortex.clp")))
          (testcase-assertion (parent boost-has-prefix0)
                              (expected TRUE)
                              (actual-value (has-prefix donuts
                                                        do)))
          (testcase-assertion (parent boost-has-suffix0)
                              (expected TRUE)
                              (actual-value (has-suffix donuts
                                                        nuts)))
          (testcase-assertion (parent boost-string-trim0)
                              (expected "donuts")
                              (actual-value (string-trim "   donuts   ")))
          (testcase-assertion (parent boost-string-trim-front0)
                              (expected "donuts   ")
                              (actual-value (string-trim-front "   donuts   ")))
          (testcase-assertion (parent boost-string-trim-back0)
                              (expected "   donuts")
                              (actual-value (string-trim-back "   donuts   ")))
          (testcase-assertion (parent boost-clamp0)
                              (expected 0)
                              (actual-value (clamp 0 -1 1)))
          (testcase-assertion (parent boost-clamp0)
                              (expected 1)
                              (actual-value (clamp 0 1 2)))
          (testcase-assertion (parent boost-clamp0)
                              (expected 2)
                              (actual-value (clamp 89 1 2))))


;TODO: add tests for the functions found in functional.cc
(deffunction MAIN::invoke-test
             ()
             )

