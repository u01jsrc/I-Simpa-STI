# -*- coding: cp1252 -*-

import uictrl as ui
import uilocale
import os
from libsimpa import *

ScriptFolder=ui.application.getapplicationpath()["userscript"]+"recp_res_advanced_tool"+os.sep
_=uilocale.InstallUiModule(ScriptFolder,ui.application.getlocale())


def GetMixedLevel(folderwxid,param):
    """
     Retourne un tableau contenant le niveau sonore global et toute bande des r�cepteurs ponctuels d'un dossier
     folderwxid identifiant wxid de l'�l�ment dossier contenant les r�cepteurs ponctuels.
    """
    cols=[]
    #folder devient l'objet dossier
    folder=ui.element(folderwxid)
    #dans un tableau on place les indices des fichiers de donn�es des r�cepteurs ponctuels
    recplist=folder.getallelementbytype(ui.element_type.ELEMENT_TYPE_REPORT_GABE_GAP)
    #Pour chaque r�cepteur on demande � l'application les donn�es trait�es du fichier (niveau sonore et cumuls)
    for idrecp in recplist:
        #recp devient l'objet ayant comme indice idrecp (entier)
        recp=ui.element(idrecp)

        #on demande le calcul des param�tres sonores
        ui.application.sendevent(recp,ui.idevent.IDEVENT_RECP_COMPUTE_ADVANCED_ACOUSTIC_PARAMETERS,{"LF":"80", "LFC":"80"})
        #on recupere l'element parent (le dossier de r�cepteur ponctuel)
        pere=ui.element(recp.getinfos()["parentid"])
        #application.sendevent(pere,idevent.IDEVENT_RELOAD_FOLDER)
        nomrecp=pere.getinfos()["label"]
        #on recupere les donn�es calcul�es
        params=ui.element(pere.getelementbylibelle('Advanced acoustic parameters'))
        #on stocke dans gridspl le tableau des niveaux de pression
        gridparam=ui.application.getdataarray(params)
        #on ajoute la colonne
        if len(cols)==0: #si le tableau de sortie est vide alors on ajoute les libell�s des lignes
            cols.append(list(zip(*gridparam))[0]) #libell� Freq et Global
        idcol=gridparam[0].index(param) #Changer le param�tre par celui pour lequel on veut la fusion
        cols.append([nomrecp]+[*list(zip(*gridparam))[idcol][1:]]) #1ere colonne, (0 etant le libell� des lignes) et [1:] pour sauter la premiere ligne
    
    cols2=list(zip(*cols[1:]))
    cols.append(['Average']+[float(sum(l))/len(l) for l in cols2[1:]])
    return cols

def SaveLevel(tab,path):
    #Creation de l'objet qui lit et ecrit les fichiers gabe
    tab=list(tab)
    print(tab)
    gabewriter=Gabe_rw(len(tab))
    labelcol=stringarray()
    for cell in tab[0][1:]:
        labelcol.append(str(cell))
    gabewriter.AppendStrCol(labelcol,"Label")
    for col in tab[1:]:
        datacol=floatarray()
        for cell in col[1:]:
            datacol.append(float(cell))
        gabewriter.AppendFloatCol(datacol,str(col[0]))
    gabewriter.Save(path)
    
def dofusion(folderwxid, path,param):
    arraydata=GetMixedLevel(folderwxid,param)
    print(arraydata)
    SaveLevel(zip(*arraydata),path+param+".gabe")
    #raffraichie l'arbre complet
    ui.application.sendevent(ui.element(ui.element(ui.application.getrootreport()).childs()[0][0]),ui.idevent.IDEVENT_RELOAD_FOLDER)

class manager:
    def __init__(self):
        self.GetMixedLevelid_all=ui.application.register_event(self.OnFusion_all)
        self.GetMixedLevelid_lf=ui.application.register_event(self.OnFusion_LF)
        self.GetMixedLevelid_lfc=ui.application.register_event(self.OnFusion_LFC)
    def getmenu(self,typeel,idel,menu):
        el=ui.element(idel)
        infos=el.getinfos()
        if infos["name"]==u"Punctual receivers":
            submenu=[]
            submenu.append((_(u"Calculate advanced parameters"),self.GetMixedLevelid_all))
            submenu.append(())
            submenu.append((_(u"Calculate LF"),self.GetMixedLevelid_lf))
            submenu.append((_(u"Calculate LFC"),self.GetMixedLevelid_lfc))
            menu.insert(0,())
            menu.insert(0,(_(u"Combine receivers advanced"),submenu))
            return True
        else:
            return False
    def OnFusion_all(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+_("Combined "),"LF80")
        dofusion(idel,grp.buildfullpath()+_("Combined "),"LFC80")
    def OnFusion_LF(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+_("Combined "),"LF80")
    def OnFusion_LFC(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+_("Combined "),"LFC80")

ui.application.register_menu_manager(ui.element_type.ELEMENT_TYPE_REPORT_FOLDER, manager())
