var hcal = require('./build/Debug/hcal');

console.log('\n\n!#----------Test results----------#!\n');
var d1 = new Date(2013, 0, 01, 08),
    d2 = new Date(2013, 1, 28, 23),
    d3 = new Date(2013, 1, 22, 23, 30),
    d4 = new Date(2013, 1, 23, 0, 30),
    d5 = new Date(2013, 0, 13, 16),
    d6 = new Date(2013, 0, 13, 18),
    d7 = new Date(2012, 11, 30, 10),
    d8 = new Date(2012, 11, 30, 11),
    d9 = new Date(2012, 11, 30),
    d10 = new Date(2013, 2, 3, 10),
    d11 = new Date(2013, 2, 3, 11),
    d12 = new Date(2013, 5, 1);


var a = hcal.createEvent(1, d1, d2, 96, 1, "Computers");
// var b = hcal.createEvent(2, d3, d4, 13, "Betsy", "Basketball");
// var c = hcal.createEvent(2, d5, d6, 13, "Reginald", "Cartography");
// var events = [a, b, c];
// var ret = hcal.testEventArray(events);
// console.log(ret[3].description());
// console.log("second event duration: " + b.duration() + " minutes");
//console.log(d3 + '\n');
//console.log(d4);

//invoke gc liberally during testing to expose memory leaks
global.gc();
hcal.insertEvent(d10, d11, 4, 2, "Hexadecimal Tax Accounting", true, function(err, evt){
    if (!err){
        console.log(evt.description() + " - " + evt.end());
    }
    else{
        console.log("insert event error: " + err);
    }
});
global.gc();
// hcal.updateEvent(35, d5, d6, 5, 1, "New title", false, function(err, evt){
//     console.log("updated event description: " + evt.description());
// });
hcal.updateEvent(83, d5, d6, 5, 1, "New title and I love being here...", false, function(err){
    if (!err){
        console.log("event updated successfully.");
    }
    else{
        console.log(err);
    }   
});
global.gc();
hcal.printCalendar(d1, d12, "month", function(fileName){
    console.log("Monthly calendar file: " + fileName);
});
global.gc();
hcal.printCalendar(d9, d12, "week", function(fileName){
    console.log("Weekly calendar file: " + fileName);
});
global.gc();
hcal.getEvents(d1, d12, function(err, evts){
    if (err){
        console.log(err);
    }
    else{
        var len = evts.length, i = 0;
        for (; i < len; ++i){
            console.log(evts[i].description() + " - " + evts[i].start());
        }
    }
});
global.gc();
//var cfg = hcal.createConfig(1, 5, 8, 20);
//console.log(notRet);