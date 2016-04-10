package.path = package.path .. ";../../lcUILua/?.lua;"

operations = require 'actions.operations'

function new_file()
	cadMdiChild = lc.CadMdiChild()
	cadMdiChild:newDocument()
	mdiArea:addSubWindow(cadMdiChild)
	cadMdiChild:showMaximized()
	cadMdiChild:viewer():autoScale()
	luaInterface:luaConnect(cadMdiChild:view(), "mousePressEvent()", "click")
end

function open_file()
	local fileName = qt.QFileDialog.getOpenFileName(
		mainWindow,
		qt.QObject.tr("Open File",  "", -1),
		qt.QString(""),
		qt.QObject.tr("dxf(*.dxf);;dwg(*.dwg)",  "", -1)
	)

	if(not fileName:isEmpty()) then
		load_file(fileName)
	end
end

function undo()
	local activeSubWindow = mdiArea:activeSubWindow()

	if(activeSubWindow ~= nil) then
		activeSubWindow:widget():undo()
	end
end

function redo()
	local activeSubWindow = mdiArea:activeSubWindow()

	if(activeSubWindow ~= nil) then
		activeSubWindow:widget():redo()
	end
end

function load_file(fileName)
	cadMdiChild = lc.CadMdiChild()
	cadMdiChild:import(fileName:toStdString())
	mdiArea:addSubWindow(cadMdiChild)
	cadMdiChild:showMaximized()

	luaInterface:luaConnect(cadMdiChild:view(), "mousePressEvent()", "click")
end

function click()
	local view = mdiArea:activeSubWindow():widget():view()
	local x = view:x()
	local y = view:y()

	operations.click(x, y)
end

--UI
mainWindow = qt.loadUi("../../lcUI/mainwindow.ui")
mainWindow:setWindowTitle(qt.QObject.tr("LibreCAD", "", -1)) -- Todo: optional arguments
mainWindow:setUnifiedTitleAndToolBarOnMac(true)

menuBar = mainWindow:menuBar()
drawMenu = menuBar:addMenuStr(qt.QString("Draw"))
lineAction = drawMenu:addActionStr(qt.QString("Line"))

mdiArea = mainWindow:findChild("centralWidget"):findChild("mdiArea")

mdiArea:setHorizontalScrollBarPolicy(0)
mdiArea:setVerticalScrollBarPolicy(0)

mainWindow:setCentralWidget(mdiArea)

new_file()

luaInterface:luaConnect(mainWindow:findChild("actionNew"), "triggered(bool)", "new_file")
luaInterface:luaConnect(mainWindow:findChild("actionOpen"), "triggered(bool)", "open_file")
luaInterface:luaConnect(mainWindow:findChild("actionUndo"), "triggered(bool)", "undo")
luaInterface:luaConnect(mainWindow:findChild("actionRedo"), "triggered(bool)", "redo")
luaInterface:luaConnect(lineAction, "triggered(bool)", "operations.create_line")

luaScript = lc.LuaScript(mdiArea)
mainWindow:addDockWidget(2, luaScript)

mainWindow:show();