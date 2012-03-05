//  Copyright John Maddock 2008.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

   static const boost::array<boost::array<T, 2>, 200> zeta_neg_data = {{
      {{ SC_(-29.912628173828125), SC_(-11411595.03734503626230312380285630491992) }}, 
      {{ SC_(-29.851711273193359375), SC_(-17491128.83255653477666248263741824395945) }}, 
      {{ SC_(-29.6310367584228515625), SC_(-29338557.80907609522626540806232745245856) }}, 
      {{ SC_(-29.580287933349609375), SC_(-30306395.15877987544763078130380383514834) }}, 
      {{ SC_(-29.449008941650390625), SC_(-30679896.21366776808601886434072241922978) }}, 
      {{ SC_(-29.0131816864013671875), SC_(-20461364.43945172423472081418662644617911) }}, 
      {{ SC_(-28.932170867919921875), SC_(-17954718.78416694210282615237096201590328) }}, 
      {{ SC_(-28.8929386138916015625), SC_(-16756279.89114728919232419374363852845816) }}, 
      {{ SC_(-28.8703212738037109375), SC_(-16073970.93038052754961877241558169058995) }}, 
      {{ SC_(-28.56868743896484375), SC_(-8043912.797659222132349302164106524597545) }}, 
      {{ SC_(-28.4122905731201171875), SC_(-4903635.075080145389499851622270349855906) }}, 
      {{ SC_(-28.3275470733642578125), SC_(-3515000.607629881325886083145603311677359) }}, 
      {{ SC_(-27.9045429229736328125), SC_(561872.9101127770815302245082844482980385) }}, 
      {{ SC_(-27.648517608642578125), SC_(1342198.470655673842508718473103589634627) }}, 
      {{ SC_(-26.98891448974609375), SC_(958967.3644061210965186556335814069245902) }}, 
      {{ SC_(-26.9762477874755859375), SC_(940695.8288692367233401584802662854004146) }}, 
      {{ SC_(-26.5942859649658203125), SC_(431764.3905797650182797008420936167894022) }}, 
      {{ SC_(-26.513599395751953125), SC_(344789.2311643767144853652064583290283269) }}, 
      {{ SC_(-26.3110713958740234375), SC_(166950.9217712466990407178286461519015362) }}, 
      {{ SC_(-26.119335174560546875), SC_(50216.50838488570401977081704289833643882) }}, 
      {{ SC_(-26.0972042083740234375), SC_(39696.07667360741023132228278408445173572) }}, 
      {{ SC_(-26.06340789794921875), SC_(24717.59361206202031383049563915428563206) }}, 
      {{ SC_(-25.97290802001953125), SC_(-9283.102630861689329279837111548500337322) }}, 
      {{ SC_(-25.800212860107421875), SC_(-52573.91752584962621396048551282302169873) }}, 
      {{ SC_(-25.7026424407958984375), SC_(-66701.3515681932512045823224337761059857) }}, 
      {{ SC_(-25.60152435302734375), SC_(-75127.68961892726687631489083928203368082) }}, 
      {{ SC_(-25.3774662017822265625), SC_(-77369.49762740646964459716518117457943474) }}, 
      {{ SC_(-25.3718852996826171875), SC_(-77211.18174801304523919999376682789300922) }}, 
      {{ SC_(-25.2124080657958984375), SC_(-69818.92283651854238898648172800794135819) }}, 
      {{ SC_(-25.113994598388671875), SC_(-63308.57776529969909975415070544651202913) }}, 
      {{ SC_(-25.1002140045166015625), SC_(-62323.27853183353019086156608655701085742) }}, 
      {{ SC_(-24.9590358734130859375), SC_(-51664.42535000520168097529147316913191884) }}, 
      {{ SC_(-24.6932125091552734375), SC_(-31670.01281816867446841516400384736191327) }}, 
      {{ SC_(-24.610179901123046875), SC_(-26064.93805025958872153777775894298753194) }}, 
      {{ SC_(-24.2069492340087890625), SC_(-5837.615323910213540582720536747728255397) }}, 
      {{ SC_(-24.1601581573486328125), SC_(-4267.760146707777974801177862963289435484) }}, 
      {{ SC_(-23.905548095703125), SC_(1791.561198109913837096109939891110951525) }}, 
      {{ SC_(-23.5498905181884765625), SC_(4871.416521314878510824761013567411455181) }}, 
      {{ SC_(-23.4835186004638671875), SC_(4975.227974192317781212488271510561925245) }}, 
      {{ SC_(-23.4303455352783203125), SC_(4984.336960568300045015617894873870024567) }}, 
      {{ SC_(-23.2656269073486328125), SC_(4689.056669026157611801515942421136275408) }}, 
      {{ SC_(-23.147945404052734375), SC_(4269.09697487404379106734373164284895236) }}, 
      {{ SC_(-23.0618305206298828125), SC_(3895.967523742781467049009764795714670535) }}, 
      {{ SC_(-22.865161895751953125), SC_(2953.311728747392678575675862332538485112) }}, 
      {{ SC_(-22.450725555419921875), SC_(1144.061952511390247866794452177364720154) }}, 
      {{ SC_(-22.216400146484375), SC_(433.4964624209467744240111502742949136417) }}, 
      {{ SC_(-22.117252349853515625), SC_(209.670669847220366780433535101515157212) }}, 
      {{ SC_(-22.0920505523681640625), SC_(159.7249542805152560864294822534577696243) }}, 
      {{ SC_(-22.0172443389892578125), SC_(27.28537205103064323405309172342888102988) }}, 
      {{ SC_(-21.4432220458984375), SC_(-374.2415260535891625043607900493959621482) }}, 
      {{ SC_(-21.4153881072998046875), SC_(-374.303271908688039818985907313585954208) }}, 
      {{ SC_(-21.36655426025390625), SC_(-371.7545846369976735832217959234696992869) }}, 
      {{ SC_(-21.1389904022216796875), SC_(-326.1669603491139591285009725186578239647) }}, 
      {{ SC_(-20.792087554931640625), SC_(-206.625063426231189233571547577406886055) }}, 
      {{ SC_(-20.7052173614501953125), SC_(-175.5791090488241874023715949426197745063) }}, 
      {{ SC_(-20.64069366455078125), SC_(-153.3071708461727420185632819734936375388) }}, 
      {{ SC_(-20.4468212127685546875), SC_(-92.67534368527936634847943709898214630585) }}, 
      {{ SC_(-20.1869373321533203125), SC_(-30.43108614924854718139530495549243305713) }}, 
      {{ SC_(-20.1699161529541015625), SC_(-27.17316397712932041916821790643340800192) }}, 
      {{ SC_(-19.694103240966796875), SC_(27.1714136640721528789798045585951196151) }}, 
      {{ SC_(-19.549198150634765625), SC_(32.29909704565499257138194367283757692887) }}, 
      {{ SC_(-19.4480419158935546875), SC_(33.67605769729398309956337071678460455377) }}, 
      {{ SC_(-19.1505031585693359375), SC_(30.51834489947975739023200586922515652717) }}, 
      {{ SC_(-19.0985546112060546875), SC_(29.23410383834503370501000988892387173246) }}, 
      {{ SC_(-19.064365386962890625), SC_(28.3146818846773107926880125501905809413) }}, 
      {{ SC_(-18.95330047607421875), SC_(25.02711851332149330549031951636533097105) }}, 
      {{ SC_(-18.7998867034912109375), SC_(20.07798417651821829598839554245992618286) }}, 
      {{ SC_(-18.2061786651611328125), SC_(3.48259567016902798768190885417275768286) }}, 
      {{ SC_(-18.171688079833984375), SC_(2.808091309863074171772985422170099249644) }}, 
      {{ SC_(-17.99382781982421875), SC_(-0.08425805633923885570999830270532256204479) }}, 
      {{ SC_(-17.9543361663818359375), SC_(-0.5968706969826661560508135615534999329391) }}, 
      {{ SC_(-17.840961456298828125), SC_(-1.822852247678475212693660825685951918909) }}, 
      {{ SC_(-17.83806610107421875), SC_(-1.849579792577253331534678618823864608689) }}, 
      {{ SC_(-17.7578449249267578125), SC_(-2.504689097176022370085233310047718350241) }}, 
      {{ SC_(-17.6391048431396484375), SC_(-3.193024470338536865812887315255732910235) }}, 
      {{ SC_(-17.46953582763671875), SC_(-3.679429462928429733583422830868284984109) }}, 
      {{ SC_(-17.329334259033203125), SC_(-3.730955850518545451525056518679296419855) }}, 
      {{ SC_(-17.2073383331298828125), SC_(-3.582528563068304439407127816770249435282) }}, 
      {{ SC_(-16.9253997802734375), SC_(-2.810296984266164459897058245930083917242) }}, 
      {{ SC_(-16.915283203125), SC_(-2.77589555245432113406083575136508432266) }}, 
      {{ SC_(-16.8818817138671875), SC_(-2.660508806128033733833130593715903076036) }}, 
      {{ SC_(-16.3989238739013671875), SC_(-0.9776305321380423960622625495871224063825) }}, 
      {{ SC_(-16.18682861328125), SC_(-0.3914840091082085155017628868619033070834) }}, 
      {{ SC_(-15.86298274993896484375), SC_(0.2113095736867095471658768228963977110005) }}, 
      {{ SC_(-15.8023357391357421875), SC_(0.2852756275882561377493711146849722900443) }}, 
      {{ SC_(-15.44889068603515625), SC_(0.5095766448261500611629872324946933071932) }}, 
      {{ SC_(-15.32804584503173828125), SC_(0.5204935020247384500134730244597097470842) }}, 
      {{ SC_(-15.28248119354248046875), SC_(0.518000024039102855822688087754595443758) }}, 
      {{ SC_(-14.95335483551025390625), SC_(0.4238642347557435114933888974408451377203) }}, 
      {{ SC_(-14.88536357879638671875), SC_(0.3933692627692705652205577653029677414393) }}, 
      {{ SC_(-14.81730365753173828125), SC_(0.3608573293373039084524243522210581723643) }}, 
      {{ SC_(-14.79173946380615234375), SC_(0.3482722720977891877868480924645011434135) }}, 
      {{ SC_(-14.5507144927978515625), SC_(0.2263366094050870980047656705259818867826) }}, 
      {{ SC_(-14.38645648956298828125), SC_(0.147084896053585110709304360793633346003) }}, 
      {{ SC_(-14.31533050537109375), SC_(0.1152816347651253287394552787556741727121) }}, 
      {{ SC_(-13.65148448944091796875), SC_(-0.07251231425417656377649950152568494966249) }}, 
      {{ SC_(-13.545276641845703125), SC_(-0.08375031877772358984694147296792966961615) }}, 
      {{ SC_(-13.04667377471923828125), SC_(-0.08613748015144290972450792644674564697467) }}, 
      {{ SC_(-13.03631877899169921875), SC_(-0.08554589764966609410422566041830102984647) }}, 
      {{ SC_(-13.0361614227294921875), SC_(-0.08553676925262460195666037008963074921075) }}, 
      {{ SC_(-12.95856761932373046875), SC_(-0.08056765309761383957595558846249995931279) }}, 
      {{ SC_(-12.5917186737060546875), SC_(-0.04916757520124342761036750118646514054843) }}, 
      {{ SC_(-12.39752960205078125), SC_(-0.03115114084163494465794707790061004080063) }}, 
      {{ SC_(-12.36646747589111328125), SC_(-0.02836586465940004481089358385830546327083) }}, 
      {{ SC_(-12.21360492706298828125), SC_(-0.01539036046556130908669688029744711232684) }}, 
      {{ SC_(-11.99034214019775390625), SC_(0.0006069904308788892543024610864850846763506) }}, 
      {{ SC_(-11.8568134307861328125), SC_(0.008147201960813915155871421057182351090437) }}, 
      {{ SC_(-11.856700897216796875), SC_(0.008152875868653809905975061985964214000197) }}, 
      {{ SC_(-11.62246036529541015625), SC_(0.01746194346204194142446845779680149340516) }}, 
      {{ SC_(-11.5703887939453125), SC_(0.01886566756877059520362077789879963609155) }}, 
      {{ SC_(-10.902614593505859375), SC_(0.0196626059012350652716117019088095189095) }}, 
      {{ SC_(-10.39686298370361328125), SC_(0.008689980413332084843117426575282850372437) }}, 
      {{ SC_(-10.16733551025390625), SC_(0.003416813948098911896742939961274332301015) }}, 
      {{ SC_(-10.0372104644775390625), SC_(0.0007176193820913189364411980055213424358445) }}, 
      {{ SC_(-9.9642963409423828125), SC_(-0.0006632811340011981503362639124151757266767) }}, 
      {{ SC_(-9.6919612884521484375), SC_(-0.004807940796456133525623000987469972333747) }}, 
      {{ SC_(-9.6801853179931640625), SC_(-0.004948106664919310929639531491612686942336) }}, 
      {{ SC_(-9.672039031982421875), SC_(-0.005043104323545521915854009513305224121704) }}, 
      {{ SC_(-9.42822933197021484375), SC_(-0.007140216297724451093935994631687254893686) }}, 
      {{ SC_(-8.95921039581298828125), SC_(-0.007434519412310381781780546556040051757663) }}, 
      {{ SC_(-8.9292163848876953125), SC_(-0.007313912761229121731761666014703562188579) }}, 
      {{ SC_(-8.92558765411376953125), SC_(-0.00729839292833941319530973378448137863064) }}, 
      {{ SC_(-8.87785434722900390625), SC_(-0.00707633883068269537335294544211513278741) }}, 
      {{ SC_(-8.46031284332275390625), SC_(-0.004074256641827071875084784818914263298709) }}, 
      {{ SC_(-8.447780609130859375), SC_(-0.003964973936847219963439195813913605287177) }}, 
      {{ SC_(-8.32862186431884765625), SC_(-0.002903415969312147961289161209278260789382) }}, 
      {{ SC_(-8.120998382568359375), SC_(-0.00103823391856569843811586237609042444109) }}, 
      {{ SC_(-8.11257076263427734375), SC_(-0.0009641405242205096512409518786467984036622) }}, 
      {{ SC_(-8.00969028472900390625), SC_(-0.8081897555380671288572993901221806428558e-4) }}, 
      {{ SC_(-7.96818065643310546875), SC_(0.000261996231792748327532018167663538399616) }}, 
      {{ SC_(-7.498992443084716796875), SC_(0.003273417189465077929314372888310860553978) }}, 
      {{ SC_(-7.46440410614013671875), SC_(0.003417551297097495653085939986968115182729) }}, 
      {{ SC_(-7.28324031829833984375), SC_(0.003973224858687943569640790429796668774442) }}, 
      {{ SC_(-7.03993511199951171875), SC_(0.004188064885842535489566897798831477388136) }}, 
      {{ SC_(-6.96289348602294921875), SC_(0.00413307218623721479271246253687857536289) }}, 
      {{ SC_(-6.710720539093017578125), SC_(0.003579650425628445602269416395217624438872) }}, 
      {{ SC_(-6.6343975067138671875), SC_(0.003311914606322736219077651823305971579526) }}, 
      {{ SC_(-6.604713916778564453125), SC_(0.003196837407418412428928538915252495489608) }}, 
      {{ SC_(-6.5267925262451171875), SC_(0.00286808681065953969739400100637205637184) }}, 
      {{ SC_(-6.5100555419921875), SC_(0.002792769359155893317313951562112124642491) }}, 
      {{ SC_(-6.38633251190185546875), SC_(0.002191129317123186105010600162934947438173) }}, 
      {{ SC_(-6.323749542236328125), SC_(0.001861047805191740695151906651937619919389) }}, 
      {{ SC_(-6.268978595733642578125), SC_(0.001560903697471362240875092595942452538086) }}, 
      {{ SC_(-6.00856876373291015625), SC_(0.5056493923745302784537384920689241524123e-4) }}, 
      {{ SC_(-5.854171276092529296875), SC_(-0.000850699827548195177814512570253393816845) }}, 
      {{ SC_(-5.8458156585693359375), SC_(-0.0008984756388001563233098481127032455383618) }}, 
      {{ SC_(-5.441573619842529296875), SC_(-0.002915026968595680785713416510699222417991) }}, 
      {{ SC_(-5.398475170135498046875), SC_(-0.003081263239844393794556192268338493619238) }}, 
      {{ SC_(-5.38677501678466796875), SC_(-0.003124321716617199710687253202017509643958) }}, 
      {{ SC_(-5.348802089691162109375), SC_(-0.003257740949901305827723740216599368435532) }}, 
      {{ SC_(-5.284323215484619140625), SC_(-0.003461160844402030020337003403034416960299) }}, 
      {{ SC_(-5.26421260833740234375), SC_(-0.003518393964166156715278378770334015232501) }}, 
      {{ SC_(-5.258718967437744140625), SC_(-0.003533499841767076804949459388722993177251) }}, 
      {{ SC_(-5.19130611419677734375), SC_(-0.003699890777792053165830070855367893096293) }}, 
      {{ SC_(-4.966538906097412109375), SC_(-0.003982393105088701410063296824875351251516) }}, 
      {{ SC_(-4.946559429168701171875), SC_(-0.00398600239489331055969397480564266110296) }}, 
      {{ SC_(-4.8982295989990234375), SC_(-0.003979612855119401843972374236087226591403) }}, 
      {{ SC_(-4.757171154022216796875), SC_(-0.003836405057552962242456530288751158127348) }}, 
      {{ SC_(-4.743566036224365234375), SC_(-0.003812665437930484329367028055222166144102) }}, 
      {{ SC_(-4.553936004638671875), SC_(-0.003299286445112839390501564610822343063568) }}, 
      {{ SC_(-4.54137516021728515625), SC_(-0.003253340207436420485572929378617465360415) }}, 
      {{ SC_(-4.5209980010986328125), SC_(-0.003175691140094301576675103590991471527218) }}, 
      {{ SC_(-4.4728069305419921875), SC_(-0.002976858185863617245397173014294301888612) }}, 
      {{ SC_(-4.24431324005126953125), SC_(-0.00175551877458019938328423578132053906906) }}, 
      {{ SC_(-4.114734649658203125), SC_(-0.0008756602845971692915754872652644095998698) }}, 
      {{ SC_(-3.9377651214599609375), SC_(0.0005075394336783163996239897113119062326783) }}, 
      {{ SC_(-3.69749927520751953125), SC_(0.002622130552658735330754289390878800389753) }}, 
      {{ SC_(-3.676990985870361328125), SC_(0.002810112977938437719471179582479423269376) }}, 
      {{ SC_(-2.9547061920166015625), SC_(0.00856184012269561519081119026118342931302) }}, 
      {{ SC_(-2.8205192089080810546875), SC_(0.009037989206198579118042330714986772360851) }}, 
      {{ SC_(-2.7686493396759033203125), SC_(0.009128821249400912985786171301727546611781) }}, 
      {{ SC_(-2.3819997310638427734375), SC_(0.007528500554786901983716620452938789863242) }}, 
      {{ SC_(-1.9204499721527099609375), SC_(-0.002637408450550919969361055380236527200873) }}, 
      {{ SC_(-1.87169349193572998046875), SC_(-0.004478507698442871056456977520107815363936) }}, 
      {{ SC_(-1.6853485107421875), SC_(-0.01331301401933966559224964767883365698475) }}, 
      {{ SC_(-1.6121981143951416015625), SC_(-0.01766825358627221799677465200009125172091) }}, 
      {{ SC_(-1.5669968128204345703125), SC_(-0.02064343048265733044842029728400047740426) }}, 
      {{ SC_(-1.4528820514678955078125), SC_(-0.02922143297753860874334243356099190516151) }}, 
      {{ SC_(-1.192826747894287109375), SC_(-0.05566943732471789169546884977433587458236) }}, 
      {{ SC_(-1.04621016979217529296875), SC_(-0.07595018260277478359527480657330720805406) }}, 
      {{ SC_(-1.0456688404083251953125), SC_(-0.07603371990103121884396103918441800305874) }}, 
      {{ SC_(-0.857705175876617431640625), SC_(-0.1096023911883116703076955371201458249853) }}, 
      {{ SC_(-0.543115675449371337890625), SC_(-0.1928664573137838612762872432285532859205) }}, 
      {{ SC_(-0.394455432891845703125), SC_(-0.2495443485845467778254047395388879044791) }}, 
      {{ SC_(-0.327823936939239501953125), SC_(-0.2799982799519977440228461605076452171571) }}, 
      {{ SC_(-0.317288100719451904296875), SC_(-0.2851488655947230334542341313298974452691) }}, 
      {{ SC_(-0.261966645717620849609375), SC_(-0.3138527949668934661932558041917113174462) }}, 
      {{ SC_(-0.2557342052459716796875), SC_(-0.317271262348402548403653041282308443369) }}, 
      {{ SC_(-0.247936725616455078125), SC_(-0.3216037424750364211708767678886883998309) }}, 
      {{ SC_(-0.088456094264984130859375), SC_(-0.4259272083317181924380711619296285238665) }}, 
      {{ SC_(-0.086042940616607666015625), SC_(-0.4277716953524606366478858744722936332152) }}, 
      {{ SC_(-0.001456916332244873046875), SC_(-0.4986633097035792982997269904612808871223) }}, 
      {{ SC_(0.034901142120361328125), SC_(-0.533338048759179364792881398709205491114) }}, 
      {{ SC_(0.088376700878143310546875), SC_(-0.5898057330494933987159364633901975183229) }}, 
      {{ SC_(0.414400041103363037109375), SC_(-1.174656654636669840597116356984374413125) }}, 
      {{ SC_(0.611247599124908447265625), SC_(-2.024132913638389303467555729734394339536) }}, 
      {{ SC_(0.644168853759765625), SC_(-2.259612667973966645670455260002849054666) }}, 
      {{ SC_(0.779320240020751953125), SC_(-3.970538544084429654769915000159638319936) }}, 
      {{ SC_(0.81612360477447509765625), SC_(-4.874770988252068737319828926058330714155) }}, 
      {{ SC_(0.890301287174224853515625), SC_(-8.54670722409884960433372346851249404364) }}
   }};
