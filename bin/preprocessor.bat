slangc.exe -E -lang hlsl ..\src\rfx\tests\preprocessor\PreprocStringify.rfx > preprocesed.slang
cl -E ..\src\rfx\tests\preprocessor\PreprocStringify.rfx > preprocesed.vs
dxc -P preprocesed.dxil ..\src\rfx\tests\preprocessor\PreprocStringify.rfx