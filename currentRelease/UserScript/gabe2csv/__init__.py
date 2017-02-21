# -*- coding: cp1252 -*-

import uilocale
import os
import uictrl as ui
import gabe2csv as g2c
from libsimpa import *

_=uilocale.InstallUiModule(ui.application.getapplicationpath()["userscript"]+"gabe2csv"+os.sep,ui.application.getlocale())
    
def Convert2CSV(folderwxid, path):
    g2c.main(['-i',str(path)])
    #reload folder
    ui.application.sendevent(ui.element(ui.element(ui.application.getrootreport()).childs()[0][0]),ui.idevent.IDEVENT_RELOAD_FOLDER)

class manager:
    def __init__(self):
        self.Convert2CSVid=ui.application.register_event(self.OnConvert2CSV)
    def getmenu(self,typeel,idel,menu):
        el=ui.element(idel)
        infos=el.getinfos()
        if(infos['label']==u'IntensityAnimation' or infos['label']== u'recepteurss'):
            return False
        if infos['typeElement'] == ui.element_type.ELEMENT_TYPE_REPORT_FOLDER:
            menu.insert(0,())
            menu.insert(0,(_(u"Convert all binary to csv"),self.Convert2CSVid))
        else:
            menu.insert(0,())
            menu.insert(0,(_(u"Convert to csv"),self.Convert2CSVid))
        return True

    def OnConvert2CSV(self,idel):
        grp=ui.e_file(idel)
        Convert2CSV(idel,grp.buildfullpath())
		
ui.application.register_menu_manager(ui.element_type.ELEMENT_TYPE_REPORT_FOLDER, manager())

ui.application.register_menu_manager(ui.element_type.ELEMENT_TYPE_REPORT_GABE, manager())
ui.application.register_menu_manager(ui.element_type.ELEMENT_TYPE_REPORT_GABE_RECP, manager())
ui.application.register_menu_manager(ui.element_type.ELEMENT_TYPE_REPORT_GABE_GAP, manager())
