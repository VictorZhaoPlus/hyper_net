import qualified Data.Time as T
import qualified Data.Char as C
import qualified System.Directory as D
import qualified System.IO as I
main = do
	putStrLn "please input module name:"
	name <- getLine
	putStrLn "please input author name:"
	author <- getLine
	t <- T.getCurrentTime
	D.createDirectoryIfMissing True (map C.toLower name)
	copyTemp name author (time t) "template/CMakeLists.txt" ((map C.toLower name) ++ "/CMakeLists.txt")
	copyTemp name author (time t) "template/Template.h" ((map C.toLower name) ++ "/" ++ (toH name))
	copyTemp name author (time t) "template/Template.cpp" ((map C.toLower name) ++ "/" ++ (toCPP name))
	copyTemp name author (time t) "template/main.cpp" ((map C.toLower name) ++ "/main.cpp")
	copyTemp (toInterface name) author (time t) "template/Interface.h" ("interface/" ++ (toH $ toInterface name))
	where
		time = T.formatTime T.defaultTimeLocale "%B %d, %Y, %I:%M %p"
		
copyTemp :: String -> String -> String -> FilePath -> FilePath -> IO()
copyTemp name author time src dst = do
	handle <- I.openFile src I.ReadMode
	contents <- I.hGetContents handle
	writeFile dst (rep contents)
	I.hClose handle
	where
		rep = (replace '@' (map C.toUpper name)).(replace '+' (map C.toLower name)).(replace '&' name).(replace '%' time).(replace '^' author)
	

replace :: Char -> String -> String -> String
replace c s [] = []
replace c s (x:xs) = if c == x then s ++ (replace c s xs) else x : (replace c s xs)
	
toH :: String -> String
toH m = m ++ ".h"

toCPP :: String -> String
toCPP m = m ++ ".cpp"

toInterface :: String -> String
toInterface m = 'I':m
