var hcal = require('./build/Release/hcal');

console.log('\n\n!#----------Test results----------#!\n');
var d1 = new Date(2013, 0, 01, 08);
var d2 = new Date(2013, 1, 28, 23);
var d3 = new Date(2013, 1, 22, 23, 30);
var d4 = new Date(2013, 1, 23, 0, 30);
var d5 = new Date(2013, 0, 13, 16);
var d6 = new Date(2013, 0, 13, 18);

// var a = hcal.createEvent(1, d1, d2, 96, "Bob", "Computers");
// var b = hcal.createEvent(2, d3, d4, 13, "Betsy", "Basketball");
// var c = hcal.createEvent(2, d5, d6, 13, "Reginald", "Cartography");
// var events = [a, b, c];
// var ret = hcal.testEventArray(events);
// console.log(ret[3].description());
// console.log("second event duration: " + b.duration() + " minutes");

// hcal.buildCalendar(ret, "month", function(fileName){
//     console.log("Monthly calendar file: " + fileName);    
// });
// hcal.buildCalendar(events, "week", function(fileName){
//     console.log("Weekly calendar file: " + fileName);
// });
console.log(d3 + '\n');
console.log(d4);
var evt = hcal.insertEvent(d3, d4, 4, 1, "Snowball Fight", true);
hcal.printCalendar(d1, d2, "month", function(fileName){
    console.log("Monthly calendar file: " + fileName);
});
//var cfg = hcal.createConfig(1, 5, 8, 20);
//console.log(notRet);