# -*- coding: cp1252 -*-
import math
import uictrl as ui
import uilocale
import os
from libsimpa import *

_=uilocale.InstallUiModule(ui.application.getapplicationpath()["userscript"]+"recp_surf_calc"+os.sep,ui.application.getlocale())

def mean(array, weight):
    avr = sum(x * y for x, y in zip(array, weight)) / sum(weight)
    return avr

def std(array, weight):
    avr = mean(array, weight)
    variance = sum(y*(x-avr)**2  for x, y in zip(array, weight)) / sum(weight)
    return math.sqrt(variance)


def GetSurfReceiverStd(filepath, time_start=0, time_end=0.1):
    """

    """
    print(filepath)

    data = rsurf_data()
    loader = rsurf_io()
    loader.Load(filepath, data)

    rs_count = data.GetRsCount()
    energy = []
    face_area = []

    for rs in range(rs_count):
        name = data.GetRsName(rs)
        print(name)

        faces_no = data.GetRsFaceCount(rs)
        timestep = data.GetTimeStep()
        energy += [0 for i in range(faces_no)]
        face_area += [0 for i in range(faces_no)]

        for face in range(faces_no):
            face_area[face] = data.ComputeFaceArea(rs, face)
            for record_no in range(data.GetFaceRecordCount(rs, face)):
                time_idx = data.GetFaceTimeStep(rs, face, record_no)
                time = time_idx * timestep
                if (time >= time_start and time <= time_end):
                    energy[face]+=data.GetFaceEnergy(rs, face, record_no)

        #var = std(energy, face_area)
        #print 10*math.log10(var/(10**-12))

        energy = filter(lambda a: a != 0, energy)
        energy = [10*math.log10(x/(10**-12)) for x in energy]
        standard_dev = std(energy, face_area)
        average = mean(energy, face_area)
        print('Total receiver area: %f' % sum(face_area))
        print('Average: %f' % average)
        print('STD: %f' % standard_dev)

        return average, standard_dev

    
def SaveStats(tab,path):
    #Creation de l'objet qui lit et ecrit les fichiers gabe
    gabewriter=Gabe_rw(len(tab))
    labelcol=stringarray()
    for label in ['Average', 'STD']:
        labelcol.append(label)
    gabewriter.AppendStrCol(labelcol,'surface_receiver_parameters')
    for col in tab:
        datacol=floatarray()
        for cell in col[1][:]:
            datacol.append(float(cell))
        gabewriter.AppendFloatCol(datacol,str(col[0]))
    gabewriter.Save(path)

def askForInput():
    label_start = _(u"Start time [s]")
    label_end = _(u"End time [s]")
    inputs = {label_start : "0",
              label_end : "10"}
    res = ui.application.getuserinput(_(u"Set start and end time"), _(u"Fill start and end time to calculate paramaters for specified range"),inputs)
    #print(res)

    start_time = -1
    end_time = -1

    if res[0]:
        start_time = float(res[1][label_start])
        end_time = float(res[1][label_end])

    return start_time, end_time
    
def docalculate_std(folderwxid, path):
    st, et = askForInput()
    if st == -1:
        return

    parsed = parse_input(str(path))
    results = []
    #print(parsed)
    for receiver in parsed:
        name = os.path.basename(os.path.dirname(receiver)) +'_'+ os.path.basename(receiver)
        arraydata=GetSurfReceiverStd(receiver, st, et)
        results.append((name, arraydata))
    SaveStats(results,path+"_std.gabe")
    #raffraichie l'arbre complet
    ui.application.sendevent(ui.element(ui.element(ui.application.getrootreport()).childs()[0][0]),ui.idevent.IDEVENT_RELOAD_FOLDER)

def parse_input(path):
    if os.path.isdir(path):
        paths = []
        freqs = os.listdir(path)
        for freq in freqs:
            fpath = os.path.join(path,freq)
            [paths.append(os.path.join(fpath,name)) for name in os.listdir(fpath) if ".csbin" in name]
        #print(paths)
        return paths
    else:
        return [path]


class manager:
    def __init__(self):
        self.calculate_std=ui.application.register_event(self.CalculateStd)

    def getmenu(self,typeel,idel,menu):
        el=ui.element(idel)
        infos=el.getinfos()
        #print(infos)

        if infos["typeElement"] == ui.element_type.ELEMENT_TYPE_REPORT_RECEPTEURSSVISUALISATION or infos["label"] == u'Surface receiver':
            submenu=[]
            submenu.append(((u"Calculate std"),self.calculate_std))
            menu.insert(0,())
            menu.insert(0,(u"Advanced calculation",submenu))
            return True

        return False

    def CalculateStd(self,idel):
        grp=ui.e_file(idel)
        docalculate_std(idel,grp.buildfullpath())
		
ui.application.register_menu_manager(ui.element_type.ELEMENT_TYPE_REPORT_RECEPTEURSSVISUALISATION, manager())
ui.application.register_menu_manager(ui.element_type.ELEMENT_TYPE_REPORT_FOLDER, manager())