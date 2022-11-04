const AWS = require('aws-sdk')
const sqs = new AWS.SQS({ region: 'us-west-2' })

module.exports = async (logStreamName, messages, rawPrefix=false) => {
  let prefix = rawPrefix
  if(prefix){
    prefix = prefix.replace(/^smart-dog-collar-app-test-(.*?)-/, '')
  }
  return await sendMessage({logStreamName, messages, prefix})
}

const sendMessage = async (item) => {
  var params = {
    MessageBody: JSON.stringify(item),
    QueueUrl: process.env.SmartDogCollarLoggerQueueUrl,
    MessageGroupId: 'smart-dog-collar-app-test-logger'
  };
  return await sqs.sendMessage(params).promise()
}