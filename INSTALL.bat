ECHO "Installation under way"
cmake -G "Visual Studio 12" -DCORE_COMPILE_STEP=ON -DCMAKE_BUILD_TYPE=Release
CMD /C msbuild Core.vcxproj /p:Configuration=Release