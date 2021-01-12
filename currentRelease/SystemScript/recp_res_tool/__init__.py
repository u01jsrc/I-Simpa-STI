# -*- coding: cp1252 -*-

import uictrl as ui
import uilocale
import os
from libsimpa import *

ScriptFolder=ui.application.getapplicationpath()["userscript"]+"recp_res_tool"+os.sep
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
    recplist=folder.getallelementbytype(ui.element_type.ELEMENT_TYPE_REPORT_GABE_RECP)
    #Pour chaque r�cepteur on demande � l'application les donn�es trait�es du fichier (niveau sonore et cumuls)
    for idrecp in recplist:
        #recp devient l'objet ayant comme indice idrecp (entier)
        recp=ui.element(idrecp)
        if (recp.getinfos()["name"]=="soundpressure" or recp.getinfos()["name"]=="Sound level"):
            #on demande le calcul des param�tres sonores
            ui.application.sendevent(recp,ui.idevent.IDEVENT_RECP_COMPUTE_ACOUSTIC_PARAMETERS,{"TR":"20;30", "EDT":"", "D":"50","C":"50;80","NC":"25"})
            #on recupere l'element parent (le dossier de r�cepteur ponctuel)
            pere=ui.element(recp.getinfos()["parentid"])
            #application.sendevent(pere,idevent.IDEVENT_RELOAD_FOLDER)
            nomrecp=pere.getinfos()["label"]
            #on recupere les donn�es calcul�es
            params=ui.element(pere.getelementbylibelle('Acoustic parameters'))
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
    SaveLevel(zip(*arraydata),path+param+".gabe")
    #raffraichie l'arbre complet
    ui.application.sendevent(ui.element(ui.element(ui.application.getrootreport()).childs()[0][0]),ui.idevent.IDEVENT_RELOAD_FOLDER)

class manager:
    def __init__(self):
        self.GetMixedLevelid_all=ui.application.register_event(self.OnFusion_all)
        self.GetMixedLevelid_t20=ui.application.register_event(self.OnFusion_t20)
        self.GetMixedLevelid_t30=ui.application.register_event(self.OnFusion_t30)
        self.GetMixedLevelid_edt=ui.application.register_event(self.OnFusion_edt)
        self.GetMixedLevelid_c50=ui.application.register_event(self.OnFusion_c50)
        self.GetMixedLevelid_c80=ui.application.register_event(self.OnFusion_c80)
        self.GetMixedLevelid_d50=ui.application.register_event(self.OnFusion_d50)
        self.GetMixedLevelid_sti=ui.application.register_event(self.OnFusion_sti)
    def getmenu(self,typeel,idel,menu):
        el=ui.element(idel)
        infos=el.getinfos()
        if infos["name"]==u"Punctual receivers":
            submenu=[]
            submenu.append((_(u"Calculate All"),self.GetMixedLevelid_all))
            submenu.append(())
            submenu.append((_(u"Calculate T20"),self.GetMixedLevelid_t20))
            submenu.append((_(u"Calculate T30"),self.GetMixedLevelid_t30))
            submenu.append((_(u"Calculate EDT"),self.GetMixedLevelid_edt))
            submenu.append(())
            submenu.append((_(u"Calculate C50"),self.GetMixedLevelid_c50))
            submenu.append((_(u"Calculate C80"),self.GetMixedLevelid_c80))
            submenu.append((_(u"Calculate D50"),self.GetMixedLevelid_d50))
            submenu.append((_(u"Calculate STI, NC25"),self.GetMixedLevelid_sti))
            menu.insert(0,())
            menu.insert(0,(_(u"Combine receivers"),submenu))
            return True
        else:
            return False
    def OnFusion_all(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+_("Combined "),"RT-20 (s)")
        dofusion(idel,grp.buildfullpath()+_("Combined "),"RT-30 (s)")
        dofusion(idel,grp.buildfullpath()+_("Combined "),"EDT (s)")
        dofusion(idel,grp.buildfullpath()+_("Combined "),"C-50 (dB)")
        dofusion(idel,grp.buildfullpath()+_("Combined "),"C-80 (dB)")
        dofusion(idel,grp.buildfullpath()+_("Combined "),"D-50 (%)")
        dofusion(idel,grp.buildfullpath()+_("Combined "),"STI, NC25")
    def OnFusion_t20(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+_("Combined "),"RT-20 (s)")
    def OnFusion_t30(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+_("Combined "),"RT-30 (s)")
    def OnFusion_edt(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+_("Combined "),"EDT (s)")
    def OnFusion_c50(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+_("Combined "),"C-50 (dB)")
    def OnFusion_c80(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+_("Combined "),"C-80 (dB)")
    def OnFusion_d50(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+_("Combined "),"D-50 (%)")
    def OnFusion_sti(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+_("Combined "),"STI, NC25")
ui.application.register_menu_manager(ui.element_type.ELEMENT_TYPE_REPORT_FOLDER, manager())
