" Vim syntax file
" Language: tegel
" Maintainer: David Stenberg
" Latest Revision:

if exists("b:current_syntax")
  finish
endif

" Header keywords
syn keyword tglKeyword arg record contained

" Types
syn keyword tglTypes bool int string contained

" Primitive values
syn keyword tglBoolean true false contained
syn match tglNumber "\d\+" contained
syn region tglString start=/\v"/ skip=/\v\\./ end=/\v"/

" Comments
syn keyword tglTodo contained TODO FIXME XXX NOTE
syn match tglComment "#.*$" contains=tglTodo

" Clusters
syn cluster tglHeaderCluster
 \ contains=tglBoolean,tglComment,tglKeyword,tglNumber,tglString,
 \ tglTypes
syn cluster tglBodyCluster contains=tglNumber

" Header and body regions
syn region tglInit start='\%^\_.'ms=s-1,rs=s-1 matchgroup=tglSectionSep
 \ end='^%%'me=e-2,re=e-2 contains=@tglHeaderCluster
 \ nextgroup=tglBody skipwhite skipempty
syn region tglBody matchgroup=tglSectionSep
 \ start='^%%' end='\%$' contains=@tglBodyCluster contained

hi def link tglBoolean Boolean
hi def link tglComment Comment
hi def link tglKeyword Keyword
hi def link tglNumber Number
hi def link tglSectionSep Todo
hi def link tglString String
hi def link tglTodo Todo
hi def link tglTypes Type

let b:current_syntax = "tegel"
