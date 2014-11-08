.. nkit C++ library documentation master file, created by
   sphinx-quickstart on Mon Oct 13 17:28:27 2014.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to nkit C++ library's documentation!
============================================

Main feature of nkit library is a dynamic typing realization for C++ with
JSON and XML support. It means that you can operate with data in C++ in the
same way as in Python or JavaScript languages.

nkit C++ library has following components:

- Dynamic typing in C++ with tables, JSON and XML support.
  With nkit library you can:
  
  - Create "dynamic" variables of following types:
  
    - Integer
    - Unsigned integer
    - Datetime
    - String
    - Boolean
    - List
    - Dictionary
    - Tables of dynamic variables. The following operations on tables are supported:
  
      - Creating tables with different column types. One column can contain dynamic data of only one of the types above.
      - Sorting tables by multiple columns
      - Grouping tables by multiple columns
      - Indexing tables by multiple columns. You can create more then one different index per table.
      - Searching and iterating on indexes
    
  - Create "dynamic" structures from JSON and serialize them to JSON
  - Create "dynamic" structures from XML

    - You can create "dynamic" structures, which are different from the structure of XML source
    - You can create multiple "dynamic" structures from one XML source.
    - You can explicitly identify those elements and attributes in XML source that you want to use for building "dynamic" data structures. Thus, it's possible to filter out unnecessary XML-data.
    - You can explicitly define "dynamic" type of scalar data, fetched from XML source.
    - With extra options you can tune some aspects of conversion from XML
    
  - Serialize "dynamic" structures to XML

    - With extra options you can tune some aspects of conversion to XML

- Simple test framework

- Logger
  
Contents:

.. toctree::
    
    install.rst
    usage/usage.rst
    info.rst

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

