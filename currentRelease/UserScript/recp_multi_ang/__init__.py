# -*- coding: cp1252 -*-
import math
import uictrl as ui
from libsimpa import *


def GetMixedLevel(folderwxid,param,enable):
    """
     Retourne un tableau contenant le niveau sonore global et toute bande des récepteurs ponctuels d'un dossier
     folderwxid identifiant wxid de l'élément dossier contenant les récepteurs ponctuels.
    """
    #cols=[]
    cols2=[]
    cols3=[]
    licznik=0
    avr=[]
    global_Std=[]
    avr_std=[]
    #folder devient l'objet dossier
    folder=ui.element(folderwxid)
    #dans un tableau on place les indices des fichiers de données des récepteurs ponctuels
    recplist=folder.getallelementbytype(ui.element_type.ELEMENT_TYPE_REPORT_GABE)
    #Pour chaque récepteur on demande à l'application les données traitées du fichier (niveau sonore et cumuls)
    for idrecp in recplist:
        #recp devient l'objet ayant comme indice idrecp (entier)
        recp=ui.element(idrecp)
        if recp.getinfos()["name"]=="angle_stats":
            licznik+=1
            gridparam=ui.application.getdataarray(recp)
            nomrecp=list(zip(*gridparam)[0][1:])
            #on ajoute la colonne
            #if len(cols)==0: #si le tableau de sortie est vide alors on ajoute les libellés des lignes
            #    cols.append(list(zip(*gridparam)[0])) #libellé Freq et Global
            if len(cols2)==0: #si le tableau de sortie est vide alors on ajoute les libellés des lignes
                cols2.append(list(gridparam[0][0:])) #libellé Freq et Global
            if len(cols3)==0: #si le tableau de sortie est vide alors on ajoute les libellés des lignes
                cols3.append(list(gridparam[0][0:])) #libellé Freq et Global
				
            for angle in gridparam[1:]:
				if len(cols2)-1<angle[1]:
					cols2.append(angle)
				else:
					cols2[int(angle[1])][2:]=[x+y for x,y in zip(cols2[int(angle[1])][2:], angle[2:])]
								
            for angle in gridparam[1:]:
				if len(cols3)-1<angle[1]:
					cols3.append(angle[0:2] + [i ** 2 for i in angle[2:]])
				else:
					cols3[int(angle[1])][2:]=[x+y for x,y in zip(cols3[int(angle[1])][2:], [i ** 2 for i in angle[2:]])]

    for row in cols2[1:]:
		row[2:]= [x/licznik for x in row[2:]]
		
    for row in cols3[1:]:
		row[2:]= [x/licznik for x in row[2:]]
		
    for row1, row2 in zip(cols2[1:], cols3[1:]):
		row2[2:]=[math.sqrt(math.fabs(y-x*x)) for x,y in zip(row1[2:],row2[2:])]
		
    for row1 in zip(*cols2[1:])[2:]:
		avr.append(sum(row1[0:])/len(row1[0:]))
    cols2.append(['srednia',0]+avr)
    
    for row1 in zip(*cols3[1:])[2:]:
		avr_std.append(sum(row1[0:])/len(row1[0:]))
    cols3.append(['srednia',0]+avr_std)
	
    return [cols2,cols3]

def CalcPerc(tab1,tab2):
    avr=[]
    JND=[]   
    JND.append(tab1[0])
    for wek1, wek2 in zip(tab1[1:-1], tab2[1:-1]):
		tmp=[]
		tmp.append(wek2[0])
		for x,y in zip(wek1[1:],wek2[1:]):
			if x==0:
				tmp.append(0)
			else:
				tmp.append(y / x)
		JND.append(tmp)
		
    for row1 in zip(*JND[1:])[2:]:
		avr.append(sum(row1[0:])/len(row1[0:]))
    JND.append(['srednia',0]+avr)
    return JND
    
def SaveLevel(tab,path,param):
    #Creation de l'objet qui lit et ecrit les fichiers gabe
    gabewriter=Gabe_rw(len(tab))
    labelcol=stringarray()
    for cell in tab[0]:
        labelcol.append(cell.encode('cp1252'))
    gabewriter.AppendStrCol(labelcol[1:],labelcol[0])
    for col in tab[1:]:
        datacol=floatarray()
        for cell in col[1:]:
            datacol.append(float(cell))
        gabewriter.AppendFloatCol(datacol,str(col[0]))
    gabewriter.Save(path.encode('cp1252'))
    
def dofusion(folderwxid, path,param,enable):
    arraydata=GetMixedLevel(folderwxid,param,enable)
    Perc=CalcPerc(arraydata[0],arraydata[1])
    SaveLevel(zip(*arraydata[0]),path+param+".gabe",param)
    SaveLevel(zip(*arraydata[1]),path+param+"_std.gabe",param)
    SaveLevel(zip(*Perc),path+param+"_std_perc.gabe",param)
    #raffraichie l'arbre complet
    ui.application.sendevent(ui.element(ui.element(ui.application.getrootreport()).childs()[0][0]),ui.idevent.IDEVENT_RELOAD_FOLDER)

class manager:
    def __init__(self):
        self.GetMixedLevelid_angle=ui.application.register_event(self.OnFusion_angle)
    def getmenu(self,typeel,idel,menu):
        el=ui.element(idel)
        infos=el.getinfos()
        if infos["name"]==u"SPPS":
			submenu=[]
			submenu.append(((u"Combine"),self.GetMixedLevelid_angle))
			menu.insert(0,())
			menu.insert(0,(u"Combine angle",submenu))
			return True
        else:
            return False
    def OnFusion_angle(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+"fusion_","angle",1)

		
ui.application.register_menu_manager(ui.element_type.ELEMENT_TYPE_REPORT_FOLDER, manager())
