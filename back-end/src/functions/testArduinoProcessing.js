const { Response } = require('./helpers');
var AWS = require("aws-sdk");
const sqs = new AWS.SQS({apiVersion: '2012-11-05'});
const messageGroupId = Date.now().toString();
exports.handler = async(event) => {
  const params = {
    MessageBody: JSON.stringify(event),
    QueueUrl: 'https://sqs.us-west-2.amazonaws.com/209462133621/smart-dog-collar-app-test-dev-ArduinoProcessedDataQueue.fifo',
    MessageGroupId: messageGroupId
  };

  try {
      const data = await sqs.sendMessage(params).promise();
      console.log(`Message sent with ID: ${data.MessageId}`);
  } catch (err) {
      console.error(err);
}
};