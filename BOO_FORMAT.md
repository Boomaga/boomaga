Boomaga files format specification
==================================

Overview
--------
This is the specification for a boomaga file format. A Boomaga file is a PJL file with extension ".boo" containing an additional PJL commands that provides information for the boomaga program.


Boomaga file structure
----------------------
The first line of a PPD file must be
```
<ESC>%-12345X@PJL BOOMAGA_PROGECT
```
&lt;ESC&gt; identifies a escape control character (ASCII 27).



Project meta info
-----------------
```
@PJL BOOMAGA META_AUTHOR="str"
```     
This command is used to specify the name of a perfomer for a document. Used when exporting to PDF.


```
@PJL BOOMAGA META_TITLE="str"
```
This command is used to specify a title for a document. Used when exporting to PDF.


```
@PJL BOOMAGA META_SUBJECT="str"
```  
This command is used to specify a subject for a document. Used when exporting to PDF.

  
```
@PJL BOOMAGA META_KEYWORDS="str"
```
This command is used to specify a keywords associated with the document. Used when exporting to PDF.


Job meta info
-------------
```
@PJL BOOMAGA JOB_TITLE="str"
```
This command is used to specify a title for a single job.



```
@PJL BOOMAGA JOB_PAGES="pagesSpec"
```
This command is used to specify the settings for the pages in the job.
A value consists of a sequence of one page options segments separated by a comma (,).

Page spec is PageNum:Hidden:Rotation:StarBooklet
 * PageNum  -  number of the page in the source PDF. If page is a inserted blank page PageNum is letter 'B'
 * Hidden   -  if page is hidden then use letter 'H', otherwise this field is empty or omitted.
 * Rotation -  One of 0,90,180,270. If rotation is 0 this field can be omitted.
 * StarBooklet- if it's first page in booklet use letter 'S', otherwise this field is empty or omitted.
 
 
Example:
```
@PJL BOOMAGA JOB_PAGES="1,2::180,B,3:H:90,4:H"
```
