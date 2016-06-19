/******************************************************************************
* The MIT License (MIT)
*
* Copyright (c) 2016 Baldur Karlsson
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
******************************************************************************/

#include "test_common.h"

std::string lipsum = R"EOLIPSUM(
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur sollicitudin congue urna, sed rhoncus magna hendrerit nec.
Fusce in mi quis sapien aliquam tempus. Pellentesque eu sagittis sapien, nec viverra velit. Morbi a leo aliquam, sagittis leo et,
imperdiet ipsum. Proin molestie lectus a urna auctor, eu aliquam diam mattis. Etiam mollis, nisl sit amet malesuada semper,
neque metus lacinia odio, a efficitur nibh eros euismod magna. Suspendisse imperdiet dui ipsum, vel blandit neque ullamcorper id.

Sed mattis mauris velit, ultrices interdum quam aliquet non. Nunc nisi felis, tristique sit amet dapibus in, egestas eu arcu.
Cras eget est augue. Pellentesque ac consequat nunc. Aenean at tellus ut dolor venenatis ultrices a sagittis lectus. Donec
vestibulum neque ut justo facilisis vestibulum. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos
himenaeos. In hac habitasse platea dictumst. Nulla tempor arcu quis lectus ultrices lobortis. Ut lobortis sem enim, ut auctor
leo tincidunt vitae. Quisque varius dui et egestas euismod. Suspendisse potenti. Praesent id libero facilisis, blandit orci ac,
consequat neque. Phasellus ac eros nec felis sagittis iaculis.

Nullam est urna, molestie ut enim non, consectetur pellentesque lorem. Phasellus efficitur, dui non efficitur molestie, justo
felis convallis mauris, quis dictum sem sapien et enim. Nam sed lacinia libero. Cras hendrerit tincidunt lacus, non interdum
risus egestas sed. Fusce tempor risus non risus vulputate, et iaculis libero hendrerit. Aenean justo mi, semper ut scelerisque
sodales, mattis vitae arcu. Sed augue mi, sollicitudin pulvinar justo et, mollis auctor neque.

Sed sed tincidunt nunc. Sed et accumsan tellus. Etiam scelerisque convallis neque, id consequat arcu ornare ut. Proin ullamcorper
nibh vel massa interdum pellentesque. Vestibulum erat lorem, sagittis et nisi non, rutrum tincidunt enim. Morbi condimentum ex
vitae ultricies mollis. Aenean dignissim, tortor nec blandit laoreet, lectus sem iaculis purus, ac elementum ipsum quam a diam.
Curabitur diam diam, dignissim quis mi non, laoreet consectetur erat. Donec sed volutpat dolor. Nam dapibus egestas nisl et
finibus. Ut in auctor mi. In condimentum, quam at porta eleifend, dolor nisl laoreet mi, vitae vulputate augue nibh vitae urna.
Aenean eu lacus in nulla dictum faucibus. Aliquam sit amet condimentum urna. Nunc eget nulla consequat est volutpat tristique.
Nulla sit amet pulvinar ligula.

Aliquam aliquet purus nec turpis posuere cursus. Suspendisse vulputate nunc urna, ac rutrum diam suscipit quis. Etiam tempor,
magna sit amet efficitur rhoncus, quam est pharetra sapien, eget fringilla turpis nisl a orci. Morbi mi turpis, bibendum ut nulla
gravida, placerat euismod leo. Etiam at turpis ipsum. Donec vel iaculis magna. Sed ac enim at neque sodales ullamcorper et nec
purus. Quisque ultricies feugiat condimentum. Nam tincidunt finibus dictum. Nulla porttitor malesuada augue in pharetra.

Quisque odio mi, convallis ac velit et, dapibus fermentum ipsum. Aliquam facilisis interdum quam, ac fringilla magna tincidunt
sed. Nulla mollis tempor tristique. Aenean gravida scelerisque nunc, ut ornare lectus porttitor nec. Donec dapibus lectus sapien,
in vehicula ex euismod id. Phasellus a mauris nunc. Integer sed vehicula enim, eget commodo justo. Vestibulum est ligula,
facilisis lacinia leo nec, accumsan mollis diam. Interdum et malesuada fames ac ante ipsum primis in faucibus. Aliquam eget
tempus lacus. In fermentum nisl vitae magna viverra efficitur. In maximus tempor nunc sit amet congue. Praesent a nisl orci.
Nulla elementum purus vitae dui rhoncus mollis.

Nam et est vitae ex convallis scelerisque. Fusce egestas blandit erat, a lobortis eros luctus non. Vestibulum ante ipsum primis
in faucibus orci luctus et ultrices posuere cubilia Curae; Proin at purus a massa tempus placerat. Nulla ultricies tellus vitae
mi consequat vulputate. Aliquam eu porttitor est. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per
inceptos himenaeos. Suspendisse aliquam lobortis luctus. Integer lacus augue, dapibus eu bibendum id, volutpat dictum massa.
Pellentesque nec consequat erat. Cras vulputate consequat euismod.

Cras dapibus, tellus finibus sollicitudin ullamcorper, leo ipsum hendrerit lorem, eget auctor metus lorem vitae elit. Phasellus
ut orci sodales, ullamcorper turpis in, interdum nisi. Nunc convallis neque a diam fringilla, at accumsan sem venenatis. Etiam
at placerat turpis. Vivamus faucibus id velit eu porta. Vestibulum faucibus iaculis nisi, vel porta quam pulvinar vitae. Integer
tincidunt rhoncus bibendum. Sed molestie eros sit amet consequat pretium. Vestibulum pharetra tempor sagittis. Mauris
consectetur urna a elit semper dignissim. Sed at metus risus. Mauris vulputate bibendum neque sit amet commodo. Integer quis
tellus vel metus egestas laoreet nec et orci. Donec pulvinar lectus vitae convallis venenatis. Nulla in tempus enim, bibendum
rutrum arcu.

Suspendisse ut convallis massa. Fusce suscipit dignissim laoreet. Integer ullamcorper lorem lobortis quam interdum accumsan.
Curabitur aliquet, magna vel condimentum condimentum, neque nisi lobortis leo, sed luctus dolor tellus id nisl. Integer at
orci sed ligula gravida imperdiet a ut velit. Vivamus posuere efficitur elit dictum aliquet. Cum sociis natoque penatibus et
magnis dis parturient montes, nascetur ridiculus mus. Nulla dictum elit eget lacus pharetra, ac consectetur purus suscipit.
Donec ultricies ut magna at convallis. Nullam eu ex ut sapien tincidunt volutpat. Nulla vel urna efficitur, finibus elit vel,
tincidunt quam. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Nam ac magna ac
augue maximus venenatis vitae vitae mi. Sed rutrum commodo imperdiet.

Proin rhoncus, ex et blandit tincidunt, mauris dolor accumsan arcu, ut feugiat ipsum arcu id libero. Nam bibendum ex nec
tortor porta, sit amet accumsan tellus interdum. Suspendisse luctus, nibh vitae egestas porta, ex diam fermentum nisi, ut
lacinia libero nulla ac velit. Nunc et mauris sed magna efficitur tempor vitae nec nisi. Sed eleifend pulvinar turpis, et
ultricies dui placerat vitae. Phasellus non diam sagittis, mattis leo sit amet, tristique massa. Aliquam fringilla sed nibh
nec vulputate. Fusce porttitor lectus nunc, iaculis consectetur felis tincidunt at. Lorem ipsum dolor sit amet, consectetur
adipiscing elit. Aenean nibh metus, placerat eu diam non, volutpat consectetur velit. Aliquam a nulla mauris. Vivamus ornare
mi arcu, id sodales risus rhoncus vitae. Mauris arcu magna, congue vel ornare eu, commodo in ante. Pellentesque non risus sit
amet dolor pharetra elementum nec quis sem. Donec eget semper ligula. Nam at augue ut elit sagittis tincidunt vitae id tortor.

Vestibulum euismod, mauris nec tristique auctor, tortor lorem hendrerit velit, quis laoreet dui ex et risus. Quisque egestas
vulputate magna vitae accumsan. Nulla eu magna elit. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi vel arcu
quis sem fringilla mattis. Duis tempus vehicula quam, nec dictum justo rutrum nec. Maecenas vulputate posuere enim, a gravida
elit mollis in. Proin elit ipsum, tincidunt a mattis vel, suscipit quis augue. Ut tristique eros libero, sed eleifend leo
volutpat ut. Mauris eu hendrerit tortor. Curabitur vitae odio quis ipsum varius dignissim sit amet at odio. Pellentesque et
laoreet justo, at gravida leo.

Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Mauris venenatis urna mi, nec finibus
mi fermentum quis. Proin bibendum, nulla sit amet mattis varius, augue mi hendrerit mauris, sed dictum ante sapien nec ex.
Sed eu iaculis dolor, in sollicitudin neque. Aenean suscipit nisl imperdiet imperdiet interdum. Curabitur lacinia vulputate
eros eget ultricies. Praesent commodo ornare erat non posuere. Praesent consectetur quis neque at dictum. Etiam gravida,
magna at vehicula eleifend, ex enim blandit lorem, cursus sagittis nibh elit eget dui. Mauris ultricies laoreet dui non
luctus. Integer egestas gravida elit ac convallis. Phasellus et cursus ante, sed iaculis enim. Fusce tempus pulvinar lectus
sed viverra. Nam non leo eget nibh convallis iaculis eu quis leo.

Curabitur et purus porttitor, vulputate tortor ac, congue ligula. Cras at nisi quis leo pharetra pharetra. Praesent bibendum
est in nunc dignissim, a eleifend diam pulvinar. Proin rhoncus, orci id aliquam gravida, nisi odio viverra diam, at placerat
odio libero ut sem. Sed in consequat augue. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac
turpis egestas. Fusce in mattis magna. Nulla non augue nec nisl bibendum tincidunt. Nulla at ipsum a est pellentesque
hendrerit. Nunc interdum dui hendrerit neque congue, quis viverra quam faucibus. Pellentesque elementum convallis quam,
vitae aliquet dui bibendum at. Nulla facilisi.

Maecenas dictum nec mi eget hendrerit. Ut purus dui, semper a ligula in, suscipit aliquet ex. Aenean pellentesque quam nisl,
laoreet ullamcorper quam feugiat vitae. In ac ultrices augue, et gravida ligula. Etiam orci risus, pellentesque nec purus et,
tristique feugiat erat. Quisque imperdiet leo nulla, id elementum tortor viverra mattis. Sed sit amet dapibus est, vitae
feugiat libero. Vestibulum molestie pharetra metus, ut fringilla turpis pharetra in. Etiam eu mauris felis. Etiam iaculis
ullamcorper massa a varius. Donec eget lacinia metus. Donec semper eget tellus sed auctor. Suspendisse nec eros nunc. Nulla
blandit dictum aliquet. Curabitur semper tellus eu pulvinar commodo. Vivamus lacus arcu, sagittis id elit non, convallis
iaculis nulla.

Aenean hendrerit, diam nec congue hendrerit, urna magna placerat dui, id ultrices sapien metus eget lectus. Cras vitae tortor
condimentum, ornare ante et, faucibus turpis. Integer sed enim purus. Morbi sed porta velit. Aliquam vestibulum nibh ligula,
a hendrerit urna gravida sit amet. Donec sodales turpis at nisl lobortis, at elementum urna commodo. Vestibulum vel mauris
id diam finibus tincidunt efficitur sit amet nunc. Curabitur sit amet ultricies purus, vitae elementum metus. Pellentesque
aliquet diam leo. Curabitur ut venenatis est. Maecenas turpis risus, venenatis sed nisl vel, elementum accumsan lorem. Praesent
ut rutrum nunc. Maecenas id egestas mi, eu ornare tortor. Nam nunc dolor, aliquam ac nunc ac, aliquet ornare lacus.

Donec rhoncus dolor nec neque consequat auctor. Phasellus in lorem commodo, dignissim metus vestibulum, auctor nisl. Phasellus
aliquam urna quis pretium pharetra. Nullam cursus auctor nisl quis vehicula. Sed tincidunt laoreet eleifend. Duis eleifend
hendrerit risus, sed bibendum eros lobortis non. Cras maximus condimentum tortor at rutrum. Aenean non vestibulum mi. Nulla
vitae varius urna. Nunc laoreet, dolor id bibendum tristique, diam tellus posuere ipsum, eget rutrum diam ligula in risus.
Aliquam vel posuere augue. Curabitur suscipit scelerisque vestibulum. In non bibendum urna.

Donec ornare, est eget accumsan vestibulum, tellus turpis consequat sem, et molestie ex ante eu lectus. Integer suscipit purus
orci, vel interdum eros elementum quis. Nunc leo purus, interdum ac sem in, porttitor sollicitudin velit. Proin efficitur id
neque eget ultrices. Vivamus aliquet blandit lectus id euismod. Suspendisse efficitur aliquet libero et accumsan. Phasellus
ullamcorper mattis sapien tincidunt condimentum. Nullam id blandit arcu. Pellentesque at ante mattis, aliquet urna vel,
viverra nisi.

Pellentesque in fermentum diam. Nulla sodales eros ut lectus bibendum elementum. Morbi aliquam, metus sed tempor blandit,
libero neque hendrerit massa, vulputate posuere augue mauris a ante. Phasellus maximus enim a finibus blandit. Donec auctor
velit sed nibh vehicula, at egestas purus rhoncus. Duis rhoncus eros ultricies, viverra dolor eget, viverra sem. Mauris
viverra elementum sapien, quis egestas turpis porta et. Quisque vestibulum maximus dolor dictum suscipit. Suspendisse tempor
lorem vel est hendrerit dictum. Interdum et malesuada fames ac ante ipsum primis in faucibus. Sed ex dui, placerat a eleifend
eget, elementum nec nibh.

Nulla porta metus non libero egestas, ut eleifend tortor posuere. Maecenas a interdum ex, sagittis congue nisi. Vivamus luctus
luctus accumsan. Etiam vel tempus ante, ac gravida tellus. Phasellus nunc enim, commodo vitae aliquam in, semper a lorem.
Pellentesque tincidunt ante orci, a maximus nulla commodo nec. Quisque posuere eget sapien et maximus. Pellentesque tempor
augue in lorem efficitur tristique. Nam eget nibh nisl. Aliquam quis purus quis diam tincidunt consequat. Donec hendrerit
augue et arcu aliquet, eu tempus urna bibendum. Integer dignissim nulla maximus leo hendrerit pharetra. Sed id consectetur
justo. Nullam tristique sit amet justo at interdum. Curabitur blandit, tortor et porttitor ullamcorper, velit tortor placerat
risus, id volutpat lectus erat sed velit. Cras ac arcu non leo lacinia maximus ut eget libero.

Sed rhoncus pharetra auctor. Maecenas quis eros ut nunc lacinia euismod. Integer ut ultricies est, id semper ante. Integer
suscipit eu mauris sed sagittis. Integer sed tortor eu ex elementum efficitur. Lorem ipsum dolor sit amet, consectetur
adipiscing elit. Donec mattis rutrum elementum. Mauris pretium nisi eget eros aliquet hendrerit. Cras libero ipsum, semper
consequat ipsum interdum, tempus efficitur enim. Maecenas mattis id enim sit amet luctus.

Pellentesque condimentum neque nulla, at rutrum turpis laoreet quis. Nulla fringilla libero in augue aliquet mattis. Duis eu
elit vitae turpis vestibulum iaculis eget nec ante. Donec tempor viverra metus, ac euismod lectus blandit ut. Proin diam
augue, suscipit ac felis ut, pretium viverra tellus. Sed faucibus magna metus, a scelerisque augue porttitor eu. Cras ac nisl
metus. Quisque dapibus urna eu magna lobortis, ut imperdiet justo eleifend. Nulla placerat eleifend erat, ac sodales ipsum.
Phasellus a quam eu tellus ultricies maximus non sodales felis. Proin in finibus mi, vel eleifend mauris. Integer convallis
malesuada nunc, sed lacinia leo aliquam a.

Cras sodales, quam eu gravida pharetra, lacus tellus aliquet ipsum, nec ultricies ex est ac sem. Morbi laoreet at nisl eu
ultrices. Cras vulputate gravida fermentum. Morbi dictum pharetra mauris ut pellentesque. Sed libero lacus, rutrum quis sem
id, tempor facilisis lectus. Etiam euismod finibus felis, at ultricies velit. Ut iaculis id justo sed luctus. Duis tincidunt
turpis sit amet sapien dapibus, id laoreet ex ullamcorper. Vestibulum sollicitudin in ligula sit amet rutrum. Praesent eget
ultrices libero.

Sed efficitur eleifend erat vitae tempor. Donec molestie vehicula erat vitae pretium. Phasellus nec odio velit. Proin quis
consectetur neque, quis vulputate enim. Suspendisse potenti. Mauris finibus ante ex, eu auctor nibh tristique malesuada. Nam
in nisi eu ex consectetur faucibus. Ut id massa vitae ex varius molestie. Nullam volutpat vitae turpis ut porttitor. Nullam
condimentum, justo vel posuere elementum, nisi nisi sollicitudin nulla, sed dignissim dui turpis eget nulla.

Praesent dignissim ornare velit, varius interdum sapien imperdiet ac. Aenean sit amet interdum libero, non porttitor est.
Donec lacus justo, vehicula a massa ut, ullamcorper fermentum lectus. Donec vel interdum diam. Fusce volutpat faucibus congue.
Sed imperdiet nisi id leo malesuada, sit amet aliquam elit tincidunt. Nunc tempor nulla augue, sed dapibus urna faucibus vel.
Proin ullamcorper consequat tellus sed consequat. Nullam mollis condimentum ullamcorper. Quisque vehicula, odio id euismod
laoreet, nibh lacus ullamcorper orci, malesuada mattis nisi sapien eget neque. In porta fermentum nulla molestie facilisis.
In hac habitasse platea dictumst. Nunc et sodales ex. Ut volutpat nisl ac efficitur dictum. Lorem ipsum dolor sit amet,
consectetur adipiscing elit.

)EOLIPSUM";

