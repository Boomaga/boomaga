About the program
=================

Boomaga (BOOklet MAnager) is a virtual printer for viewing a document before printing it out using the
physical printer.

The program is very simple to work with. Running any program, click “print” and select “Boomaga” to
see in several seconds (CUPS takes some time to respond) the Boomaga window open. If you print out
one more document, it gets added to the previous one, and you can also print them out as one.

Regardless of whether your printer supports duplex printing or not, you would be able to easily print on
both sides of the sheet. If your printer does not support duplex printing, point this out in the settings,
and Booklet would ask you to turn over the pages half way through printing your document.

The program can also help you get your documents prepared a bit before printing. At this stage
Boomaga makes it possible to:
* Paste several documents together.
* Print several pages on one sheet.
    * 1, 2, 4, 8 pages per sheet
    * Booklet. Folding the sheets in two, you’ll get a book.

Boomaga is an open source project distributed under the GPLv2 license (some files are distributed
under the LGPLv2+ license). It would be more convenient to install the program from the package for your
distribution; you can access the list of the packages at [our site](https://github.com/Boomaga/boomaga). You can also build the program from the sources; you can download the sources of the latest stable version here. 
The version for developers is available on our page at [github](https://github.com/Boomaga/boomaga).

Address your preferences and error messages to our [Issue tracker](https://github.com/Boomaga/boomaga/issues).


Why you may need it?
====================

Instance 1
----------

Think of all the times you were getting on paper something different than you expected. You may once
have forgotten to give the number of pages in the print box, or a document from your office program
did not fit the sheet. Boomaga makes possible previewing before actual printing to see the real way the
final version would look like.

Instance 2
----------

Let’s say you wish to print out and read a few documents in peace and quiet. The conventional printout
produces several A4 sheaves. They are awkward to read and store. Boomaga gets you one compact A5
booklet. It is more convenient to read than the A4 format and it fits snugly into a bookshelf and takes
much less paper.

Instance 3 (Don't try this at home)
-----------------------------------

You have an exam to take and you feel like you are all at sea. Boomaga offers a layout of 8 pages per
sheet (8Up) enabling you to print crib notes for your upcoming exam.

  **Disclaimer**
  The program developers give no guarantees and decline all responsibility for your failure or
  success.

How it Works
============

Boomaga is comprised of a backend for CUPS, and a graphic program for the viewing and editing of
documents.

A document printed out with Boomaga gets into CUPS. CUPS creates a PostScript file and passes it on
to the backend. In this instance this is a backend for Boomaga. The backend seeks out an active session
for the user who sends the document for getting printed. Via the D-Bus, it subsequently triggers the GUI
part of Boomaga (unless it already runs) and communicates to it the name of the PostScript file. GUI
scans the PostScript file and displays its content. Used for this purpose is the GHostScript library.

Nothing works!
==============

Our backend-е for CUPS uses a search for user session; it is yet to be completely debugged and may
not work in some environments. Please, look through the error messages in CUPS logs, and send in
discovered bugs to [Issue tracker](https://github.com/Boomaga/boomaga/issues), or contact the developers.


Instalation
===========

One simple solution is to install the program from the packets for your distribution kit. Access the list of packets at our site.

Should you wish to build the program from the sources, please refer to INSTALL.txt for more information.


Installing the printer
======================

Once the program has been installed, it’s time to add the virtual printer. Please note that this has to be
done only once. You don’t have to install the printer over again when updating the program.

The best solution is to use the _scripts/installPrinter.sh_ from sources tree– just run it.

If you wish to install the printer on your own, select "_Local Printers: Boomaga (Virtual boomaga printer)_" in the "create printer" dialogue and indicate the type, "_Generic Virtual boomaga printer_" or, if you can’t access the previous, select the "_build/cups/boomaga.ppd_" PPD file.

For more information, see https://github.com/Boomaga/boomaga/wiki/Instalation page.
