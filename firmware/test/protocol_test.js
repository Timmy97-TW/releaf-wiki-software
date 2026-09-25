// Feed the firmware's real output into the page's parser.
const fs=require("fs");
// The interface lives at Digital Twin/01_UI/ in the working project and at
// ui/ in the published repository. Try both so this test runs in either.
const path=require("path");
const candidates=[path.join(__dirname,"..","..","01_UI","0924UI.html"),
                  path.join(__dirname,"..","..","ui","0924UI.html")];
const uiPath=candidates.find(fs.existsSync);
if(!uiPath) throw new Error("0924UI.html not found in: "+candidates.join(" | "));
const html=fs.readFileSync(uiPath,"utf8");
const js=html.slice(html.indexOf("<script>")+8, html.lastIndexOf("</script>"));
// run the page's IIFE with a DOM stub, then reach into it via a hook
const stubEl=new Proxy({},{get:(t,k)=>{
  if(k==="style")return{};
  if(k==="dataset")return{};
  if(k==="classList")return{toggle(){},add(){},remove(){}};
  if(k==="className"||k==="textContent"||k==="innerHTML"||k==="value")return "";
  if(k==="hidden")return false;
  if(k==="getBoundingClientRect")return()=>({width:0,height:0});
  return ()=>stubEl;
},set:()=>true});
const doc={querySelector:()=>stubEl,querySelectorAll:()=>[],addEventListener(){},createElement:()=>stubEl,
  documentElement:stubEl,body:stubEl,activeElement:null};
global.document=doc; global.window={__hook:null};
global.localStorage={getItem:()=>null,setItem(){}};
global.fetch=()=>Promise.reject(new Error("offline"));
global.navigator={};
global.setInterval=()=>0; global.setTimeout=()=>0; global.clearTimeout=()=>0;
global.TextEncoder=require("util").TextEncoder; global.TextDecoder=require("util").TextDecoder;
global.getComputedStyle=()=>({getPropertyValue:()=>"#000"});
global.Blob=function(){}; global.URL={createObjectURL:()=>"",revokeObjectURL(){}};
// expose internals: append a hook to the IIFE body
const hooked=js.replace(/\n\}\)\(\);\s*$/, "\nwindow.__hook={S:S,CH:CH,acceptLine:acceptLine,derive:derive,evalAlarms:evalAlarms,MEASURED:MEASURED,liveIn:liveIn};\n})();\n");
if(hooked===js) throw new Error("could not attach hook");
eval(hooked);
const H=window.__hook, S=H.S;
S.mode="live"; S.link.on=true; S.runStart=Date.now();

const lines=fs.readFileSync(process.argv[2],"utf8").split("\n").filter(l=>l&&!l.startsWith("#"));
let fails=[];
function expect(name,cond,got){ if(!cond) fails.push(name+"  got: "+JSON.stringify(got)); }

for(const l of lines) H.acceptLine(l);
H.derive();

// 1 every channel parsed
expect("OD parsed",  Math.abs(S.pv.od-1.1963)<1e-3, S.pv.od);
expect("flow parsed",S.pv.flow===0, S.pv.flow);
expect("PT1 parsed", Math.abs(S.pv.pt1-3.08)<1e-6, S.pv.pt1);
expect("TMP derived",Math.abs(S.pv.tmp-((3.08+1.79)/2-(0.32+0.44)/2)*0.0689476)<1e-6, S.pv.tmp);
// 2 board state from ST
expect("board valve", S.board.valve==="OPEN", S.board.valve);
expect("board light", S.board.light==="GREEN", S.board.light);
expect("board lock",  S.board.lock==="NONE", S.board.lock);
expect("board uptime",S.board.upS>0, S.board.upS);
// 3 EV lines logged
const ev=S.audit.filter(a=>a.who==="board");
expect("EV logged", ev.length>=3, ev.length);
expect("EV text kept", ev.some(a=>/IL-1 TMP/.test(a.why)), ev.map(a=>a.why));
// 4 interlock transitions logged
const il=S.audit.filter(a=>a.who==="interlock");
expect("interlock logged", il.length>=2, il.map(a=>a.what));
// 5 count of parsed lines
expect("all lines counted", S.link.inN===lines.length, S.link.inN+" of "+lines.length);

// 6 a dropped channel must clear, not freeze
H.acceptLine("PV OD=0.5 FLOW=100");
expect("missing pH cleared", S.pv.ph===null, S.pv.ph);
expect("present OD kept", S.pv.od===0.5, S.pv.od);
// 7 junk must not throw or corrupt
["","PV","PV OD=","PV OD=abc","ZZ 1=2","ST","EV"].forEach(j=>H.acceptLine(j));
expect("junk left OD alone", S.pv.od===0.5, S.pv.od);
// 8 interlock raises a P1 alarm
S.board.lock="TMP"; H.evalAlarms();
expect("interlock alarms", S.alarms.some(a=>a.id==="lock"&&a.pri===1), S.alarms.map(a=>a.id));

console.log(fails.length? "FAIL\n  "+fails.join("\n  ") : "PASS  "+lines.length+" firmware lines, 15 checks");
process.exit(fails.length?1:0);
