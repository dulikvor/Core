ECHO "Installation under way"
cmake -G "Visual Studio 12" -DCORE_COMPILE_STEP=ON
CMD /C msbuild Core.vcxproj /p:Configuration=Release