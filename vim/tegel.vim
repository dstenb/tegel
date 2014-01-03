" Vim syntax file
" Language: tegel
" Maintainer: David Stenberg
" Latest Revision:

if exists("b:current_syntax")
  finish
endif

" Header keywords
syn keyword tglSharedKeyword and or not in contained

syn keyword tglHeaderKeyword arg record contained

syn keyword tglHeaderItemKeyword cmd info default contained

syn keyword tglControlKeyword with if elif else
 \ for endif endfor in create contained

" Types
syn keyword tglTypes bool int string contained

" Functions
syn keyword tglFunction filter map contained

" Primitive values
syn keyword tglBoolean true false contained
syn match tglNumber "\d\+" contained
syn region tglString start=/\v"/ skip=/\v\\./ end=/\v"/

" Comments
syn keyword tglTodo contained TODO FIXME XXX NOTE
syn region tglComment start="#" end="$" contains=tglTodo
syn region tglBodyComment start="{#" end="#}" contains=tglTodo

syn match tglContinue "\\$" contained

syn region tglHeaderItem start='{' end='}'
 \ contains=@tglHeaderCluster,tglHeaderItemKeyword

"
syn cluster tglExpression contains=tglBoolean,tglNumber,tglString,tglTypes,
 \ tglFunction,tglSharedKeyword

" Clusters
syn cluster tglHeaderCluster
 \ contains=tglComment,tglHeaderKeyword,tglHeaderItem,@tglExpression

syn cluster tglControlCluster
 \ contains=tglControlKeyword,tglContinue,tglSharedKeyword,@tglExpression

syn cluster tglInlineCluster
 \ contains=@tglExpression

syn cluster tglBodyCluster contains=tglControl,tglInline,tglBodyComment

" TODO: handle multiple lines
syn region tglControl matchgroup=tglControlSep start='^[ \t]*%'ms=e end='$'
 \ contains=@tglControlCluster

syn region tglInline matchgroup=tglInlineSep start='{{' end='}}'
 \ contains=@tglInlineCluster

" Header and body regions
syn region tglHeader start='\%^\_.'ms=s-1,rs=s-1 matchgroup=tglSectionSep
 \ end='^%%'me=e-2,re=e-2 contains=@tglHeaderCluster
 \ nextgroup=tglBody skipwhite skipempty
syn region tglBody matchgroup=tglSectionSep
 \ start='^%%' end='\%$' contains=@tglBodyCluster contained

hi def link tglBoolean Boolean
hi def link tglBodyComment Comment
hi def link tglComment Comment
hi def link tglContinue Special
hi def link tglControlKeyword Keyword
hi def link tglControlSep Special
hi def link tglHeaderItemKeyword Keyword
hi def link tglHeaderKeyword Keyword
hi def link tglInlineSep Typedef
hi def link tglFunction Function
hi def link tglNumber Number
hi def link tglSectionSep Special
hi def link tglSharedKeyword Keyword
hi def link tglString String
hi def link tglTodo Todo
hi def link tglTypes Type

let b:current_syntax = "tegel"
