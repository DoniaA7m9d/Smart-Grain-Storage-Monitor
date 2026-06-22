function doGet(e) { 
  Logger.log(JSON.stringify(e)); // لغرض الفحص ومتابعة الأخطاء
  
  if (e == undefined) {
    return ContentService.createTextOutput("No Data Received");
  }
  
  // فتح ملف السبريدشيت الحالي واختيار أول ورقة (Sheet1)
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
  
  // جلب البيانات المرسلة من الـ ESP32 عبر الرابط (GET Parameters)
  var t1 = e.parameter.t1;
  var h1 = e.parameter.h1;
  var t2 = e.parameter.t2;
  var h2 = e.parameter.h2;
  var t3 = e.parameter.t3;
  var h3 = e.parameter.h3;
  var avgT = e.parameter.avgT;
  var avgH = e.parameter.avgH;
  
  // تسجيل الوقت والتاريخ الحالي في مصر/المخزن تلقائياً عند وصول القراءة
  var d = new Date();
  var currentTime = Utilities.formatDate(d, "GMT+3", "yyyy-MM-dd HH:mm:ss"); // توقيت مصر GMT+3
  
  // إضافة سطر جديد بأسفل الجدول يحتوي على كافة القراءات بالترتيب
  sheet.appendRow([currentTime, t1, h1, t2, h2, t3, h3, avgT, avgH]);
  
  // إرجاع رسالة نجاح للـ ESP32 ليتأكد أن البيانات وصلت وتم حفظها
  return ContentService.createTextOutput("Success. Data Saved.");  
}
