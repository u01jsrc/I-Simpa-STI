#----------------------------------------------------------------------
# I-SIMPA (http://i-simpa.ifsttar.fr). This file is part of I-SIMPA.
#
# I-SIMPA is a GUI for 3D numerical sound propagation modelling dedicated
# to scientific acoustic simulations.
# Copyright (C) 2007-2014 - IFSTTAR - Judicael Picaut, Nicolas Fortin
import os
import sys, getopt, fnmatch
import libsimpa as ls

def GabeToCsv(filepath,csvpath):
    """
        Converti un fichier GABE (Generic Array Binary Exchange) en format CSV (Comma Separated Values)
    """
    # Instanciation du lecteur
    reader=ls.Gabe_rw()
    # Lecture du fichier gabe
    if reader.Load(filepath):
        data=reader.ToList()
        if data == []:
            print("Data table is empty. Check if path to binary file is correct!")
            return
        # Rotation des données (les colonnes deviennent des lignes)
        data=zip(*data)
        # Remove unwanted characters
        data[0] = [word.replace("\n", "_") for word in data[0]]
        # Ecriture des données
        fich=open(csvpath,'w')
        for line in data:
            firstcol=True
            for col in line:
                if not firstcol:
                    fich.write(",")
                else:
                    firstcol=False
                fich.write(str(col))    # Ecriture de la cellule et virgule
            fich.write("\n")            # Retour à la ligne
        fich.close()
        print("File converted sucesfully")

# Help when used from command line with -h flag
def help():
    print("Usage: test.py -i <inputfile/path> -o <outputfile> -e <extensions>")
    print("Avalible extensions: '.recp','.recps','.gab','.gabe'")
    print("Path cannot end with '\\' character. Use '\\' '/' or '\ '")
    print("When -i is folder all subfolders will be scanned for files with extension -e (by default all supported extensions) and -o flag will be ommited")

def main(argv):   
    #Parse input argumets
    filepath = ''
    csvpath = ''
    ext = ''
    try:
        opts, args = getopt.getopt(argv,"h+i:o:e:",["help","ifile=","ofile=","extension="])
    except getopt.GetoptError:
        help()
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("-h", "--ifile"):
            help()
        elif opt in ("-i", "--ifile"):
            filepath = arg
            if filepath=='':
                help()
        elif opt in ("-o", "--ofile"):
            csvpath = arg
        elif opt in ("-e", "--extension"):
            ext = arg.split()
 
    basepath,file = os.path.split(filepath)

    #Check if inputfile is a directory - process directory
    if file==' ' or file == '' or os.path.splitext(file)[1]=='':
        #Check if extensions were provided if not use default
        if ext == '':
            ext = ['.recp','.recps','.gap','.gabe']
        #Scan for matching files
        matches = []
        for root, dirnames, filenames in os.walk(basepath):
            for filename in filenames:
                if filename.endswith(tuple(ext)):
                    matches.append(os.path.join(root, filename))
        #Process all matching files
        for filepath in matches:
            basepath,file = os.path.split(filepath)
            base = os.path.splitext(file)[0]
            csvpath = os.path.join(basepath,base + ".csv")         

            print 'Input file is :', filepath
            print 'Output file is :', csvpath

            GabeToCsv(filepath,csvpath)

    #Process if input is a single file
    else:
        #Check if file folder was specified if not use same dir and name with .csv extension
        if csvpath == '':
            basepath,file = os.path.split(filepath)
            base = os.path.splitext(file)[0]
            file = base + ".csv"
            csvpath = os.path.join(basepath,file)         

        print 'Input file is :', filepath
        print 'Output file is :', csvpath

        GabeToCsv(filepath,csvpath)

if __name__ == "__main__":
   main(sys.argv[1:])
