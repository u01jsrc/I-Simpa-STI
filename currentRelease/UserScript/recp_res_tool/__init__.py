# -*- coding: cp1252 -*-

import uictrl as ui
from libsimpa import *


def GetMixedLevel(folderwxid,param):
    """
     Retourne un tableau contenant le niveau sonore global et toute bande des récepteurs ponctuels d'un dossier
     folderwxid identifiant wxid de l'élément dossier contenant les récepteurs ponctuels.
    """
    cols=[]
    #folder devient l'objet dossier
    folder=ui.element(folderwxid)
    #dans un tableau on place les indices des fichiers de données des récepteurs ponctuels
    recplist=folder.getallelementbytype(ui.element_type.ELEMENT_TYPE_REPORT_GABE_RECP)
    #Pour chaque récepteur on demande à l'application les données traitées du fichier (niveau sonore et cumuls)
    for idrecp in recplist:
        #recp devient l'objet ayant comme indice idrecp (entier)
        recp=ui.element(idrecp)
        if recp.getinfos()["name"]=="soundpressure":
            #on demande le calcul des paramètres sonores
            ui.application.sendevent(recp,ui.idevent.IDEVENT_RECP_COMPUTE_ACOUSTIC_PARAMETERS,{"TR":"20;30", "EDT":"", "D":""})
            #on recupere l'element parent (le dossier de récepteur ponctuel)
            pere=ui.element(recp.getinfos()["parentid"])
            #application.sendevent(pere,idevent.IDEVENT_RELOAD_FOLDER)
            nomrecp=pere.getinfos()["label"]
            #on recupere les données calculées
            params=ui.element(pere.getelementbylibelle('acoustic_param'))
            #on stocke dans gridspl le tableau des niveaux de pression
            gridparam=ui.application.getdataarray(params)
            #on ajoute la colonne
            if len(cols)==0: #si le tableau de sortie est vide alors on ajoute les libellés des lignes
                cols.append(list(zip(*gridparam)[0])) #libellé Freq et Global
            idcol=gridparam[0].index(param) #Changer le paramètre par celui pour lequel on veut la fusion
            cols.append([nomrecp]+list(zip(*gridparam)[idcol][1:])) #1ere colonne, (0 etant le libellé des lignes) et [1:] pour sauter la premiere ligne
	
    cols2=zip(*cols[1:])
    cols.append(['average']+[float(sum(l))/len(l) for l in cols2[1:]])
    return cols

def SaveLevel(tab,path):
    #Creation de l'objet qui lit et ecrit les fichiers gabe
    gabewriter=Gabe_rw(len(tab))
    labelcol=stringarray()
    for cell in tab[0][1:]:
        labelcol.append(cell.encode('cp1252'))
    gabewriter.AppendStrCol(labelcol,"TR-30")
    for col in tab[1:]:
        datacol=floatarray()
        for cell in col[1:]:
            datacol.append(float(cell))
        gabewriter.AppendFloatCol(datacol,str(col[0]))
    gabewriter.Save(path.encode('cp1252'))
    
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
    def getmenu(self,typeel,idel,menu):
        el=ui.element(idel)
        infos=el.getinfos()
        if infos["name"]==u"Récepteurs_Ponctuels" or infos["name"]==u"Punctual_receivers":
			submenu=[]
			submenu.append(((u"Calculate All"),self.GetMixedLevelid_all))
			submenu.append(())
			submenu.append(((u"Calculate T20"),self.GetMixedLevelid_t20))
			submenu.append(((u"Calculate T30"),self.GetMixedLevelid_t30))
			submenu.append(((u"Calculate EDT"),self.GetMixedLevelid_edt))
			submenu.append(())
			submenu.append(((u"Calculate C50"),self.GetMixedLevelid_c50))
			submenu.append(((u"Calculate C80"),self.GetMixedLevelid_c80))
			submenu.append(((u"Calculate D50"),self.GetMixedLevelid_d50))
			menu.insert(0,())
			menu.insert(0,(u"Combine receivers",submenu))
			return True
        else:
            return False
    def OnFusion_all(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+"fusion_","RT-20 (s)")
        dofusion(idel,grp.buildfullpath()+"fusion_","RT-30 (s)")
        dofusion(idel,grp.buildfullpath()+"fusion_","EDT (s)")
        dofusion(idel,grp.buildfullpath()+"fusion_","C-50 (dB)")
        dofusion(idel,grp.buildfullpath()+"fusion_","C-80 (dB)")
        dofusion(idel,grp.buildfullpath()+"fusion_","D-50 (%)")
    def OnFusion_t20(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+"fusion_","RT-20 (s)")
    def OnFusion_t30(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+"fusion_","RT-30 (s)")
    def OnFusion_edt(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+"fusion_","EDT (s)")
    def OnFusion_c50(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+"fusion_","C-50 (dB)")
    def OnFusion_c80(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+"fusion_","C-80 (dB)")
    def OnFusion_d50(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+"fusion_","D-50 (%)")
ui.application.register_menu_manager(ui.element_type.ELEMENT_TYPE_REPORT_FOLDER, manager())
