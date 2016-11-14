# -*- coding: utf-8 -*-

geo.open(0, 1, u'geomagic-tmp-input.ply')

geo.mesh_doctor("smallcompsize", 0.00010493, "smalltunnelsize", 5.246e-005, "holesize", 5.246e-005, "spikesens", 50, "spikelevel", 0.5, "defeatureoption", 2, "fillholeoption", 2, "autoexpand", 2, "operations", "IntersectionCheck+", "SmallComponentCheck+", "Update", "Auto-Repair")

geo.clear_all()
geo.select_by_area(50)
geo.delete_triangles()

geo.saveas(u'geomagic-tmp-output.ply', 11, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, -1, 0, 1, 0)
