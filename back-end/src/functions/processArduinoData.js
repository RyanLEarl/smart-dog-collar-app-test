const { Response } = require('./helpers');
var AWS = require("aws-sdk");

exports.handler = function(event, context) {
  var eventText = `X-value: ${event.body.xRotationalAxis} \nY-value: ${event.body.yRotationalAxis} \nZ-value: ${event.body.zRotationalAxis}`;
  console.log("Received event:", eventText);
  var sns = new AWS.SNS();
  var params = {
      Message: eventText, 
      Subject: "Test SNS From Lambda",
      TopicArn: "arn:aws:sns:us-west-2:209462133621:smart-dog-collar-app-test-snsTopic:73840ec3-2a42-41c4-9fec-7c3e92891a87"
  };
  sns.publish(params, context.done);
};