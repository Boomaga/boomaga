# Boomaga features

Boomaga is a virtual printer and print-preparation application. It sits between
the application that creates a document and the physical printer, allowing the
document to be previewed, rearranged, and imposed before it is printed.

## Document assembly and page editing

Several print jobs or PDF files can be collected in one Boomaga project and
printed or exported as a single document. Boomaga can:

* Reorder complete jobs by dragging them in the job list.
* Rename, clone, or remove jobs.
* Rotate an individual page or every page in a job.
* Delete pages, restore deleted pages, or delete all pages from a selected page
  to the end of its job.
* Insert blank pages before or after a selected page.
* Preview the resulting sheets before printing.

These are page-level operations. Boomaga does not edit text, images,
annotations, links, or forms inside a PDF. It is a print-preparation and page
imposition tool rather than a general-purpose PDF content editor.

## Page layouts

Boomaga can place the following numbers of document pages on each side of a
physical sheet:

* 1 page per sheet.
* 2 pages per sheet.
* 4 pages per sheet, with horizontal or vertical ordering.
* 8 pages per sheet, with horizontal or vertical ordering.
* Booklet layout.

The preview shows the imposed sheets that will be sent to the printer or
written to the exported PDF.

## Booklets and sub-booklets

Booklet layout rearranges pages into the order needed to print, fold, and bind
a booklet. Boomaga can add the blank pages required to complete the layout.

Long documents can be divided into smaller sub-booklets. The maximum number
of sheets in each sub-booklet can be configured, and a page can be marked
manually as the beginning of a new sub-booklet. Right-to-left page direction
is also supported.

## Duplex printing

For a printer with an automatic duplexer, Boomaga can request two-sided
printing with long-edge or short-edge flipping.

For a printer without an automatic duplexer, Boomaga supports assisted manual
duplex printing. It prints one set of sides, asks the user to turn over and
reinsert the sheets, and then prints the other sides. Printer profiles can use
manual duplex printing with or without reversing the second pass to match the
paper handling of a particular printer.

## Printing options and printer profiles

Boomaga can print to any physical printer available through CUPS. Its printing
options include:

* Multiple copies and collated copies.
* Forward or reverse print order.
* Automatic, color, or grayscale output when supported by the printer.
* Configurable top, bottom, left, right, and internal margins.
* Optional borders around imposed pages.

Multiple profiles can be saved for each physical printer, making it possible
to keep different margin, duplex, color, and ordering settings for different
types of jobs.

## Projects, input, and output

Documents can be sent to Boomaga through its CUPS virtual printer. PDF files
and saved Boomaga projects can also be opened or dragged into the application
directly. Sending additional documents to the virtual printer while Boomaga is
open adds them to the current project.

A project can be saved as a `.boo` file and reopened later. Boomaga can also
autosave recent printing sessions.

The finished sheets can be:

* Printed through a selected physical printer.
* Exported as a PDF.

PDF export writes the imposed result shown in the sheet preview. For example,
when booklet or two-pages-per-sheet layout is selected, the exported PDF
contains those arranged sheets rather than an unchanged copy of the input PDF.
