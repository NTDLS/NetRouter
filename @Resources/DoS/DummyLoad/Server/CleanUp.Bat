@Echo Off

attrib -s -h -r

Del *.Ncb
Del *.Opt
Del *.Plg
Del *.Aps
Del *.Scc
Del *.suo
Del *.xml
Del *.old
Del *.user
Del *.sdf

DeleteTree Win32 /Y /DelRoot
DeleteTree IPCH /Y /DelRoot

Cls