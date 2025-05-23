#!/usr/bin/env python3

"""
Script to take a Reigster Assistant spreadsheet in xlsx format and export
each sheet as a CSV.
"""

import os
import argparse
import openpyxl
import csv


def writeCSV(output_filename, sheet_data):
    """Write out the CSV file from a single sheet's data.

    Args:
        output_filename (string): the name of the CSV file to write out
        sheet_data (list-of-lists): The sheet data as a list of rows.
    """
    with open(output_filename, 'wt', newline='') as csvfile:
        csvwriter = csv.writer(csvfile)
        for row in sheet_data:
            csvwriter.writerow(row)


def writeAllCSVs(sheets, odir):
    # At this point the data structure 'sheets' is a
    # dict-of-lists(rows)-of-lists(columns)
    # iterate over the sheets and write out each CSV
    for sheetname, sheet in sheets.items():
        output_filename = "%s.csv" % (sheetname)
        output_filename_full = os.path.join(odir, output_filename)
        print("Writing", output_filename, "...")
        writeCSV(output_filename_full, sheet)


def extractDataFromSheets(wb):
    """
    Iterate over the sheets in the workbook, extracting
    the data into a list of lists
    """
    sheets = dict()
    for worksheet in wb:
        # worksheet is a single sheet in the workbook
        sheet_rows = list()
        for row in worksheet.rows:
            # Slightly strange list comprehension is because
            # openpyxl interprets TRUE as a bool
            sheet_row = [cell.value
                         if cell.value is not True else "TRUE"
                         for cell in row]
            sheet_rows.append(sheet_row)
        sheets[worksheet.title] = sheet_rows
    return sheets


def exportCSV(filename):
    """
    Read in an Excel file and export each sheet as a CSV into the
    same directory
    """
    # read in the file
    wb = openpyxl.load_workbook(filename)
    workingdir = os.path.dirname(os.path.abspath(filename))
    sheets = extractDataFromSheets(wb)
    writeAllCSVs(sheets, workingdir)


def main():
    parser = argparse.ArgumentParser(
        description="Convert a Register Assistant spreadsheet into CSV files.")
    parser.add_argument('input_file')
    # TODO: Add an argument to allow substitition over CSVs
    args = parser.parse_args()
    exportCSV(args.input_file)


if __name__ == "__main__":
    main()
